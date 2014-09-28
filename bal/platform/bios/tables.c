/* Copyright © 2014, Shikhin Sethi
 * 
 * Permission to use, copy, modify, and/or distribute this software for any 
 * purpose with or without fee is hereby granted, provided that the above 
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH 
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, 
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <bal/tables.h>
#include <bal/mmap.h>
#include <gd_common.h>
#include <stdbool.h>
#include <string.h>

gd_rsdt_pointer_table rsdt_pointer = { .header = { .id = GD_RSDT_POINTER_TABLE_ID } };
gd_pc_pointer_table pc_pointer = { .header = { .id = GD_PC_POINTER_TABLE_ID } };

/*! Checksum a table, and return the checksum. */
static uint8_t checksum_table(const uint8_t *table, size_t length)
{
    uint8_t checksum = 0;
    for (size_t i = 0; i < length; i++) checksum += table[i];
    return checksum;
}

static void search_region(const uint8_t *region, size_t length, bool rsdp_search, 
                          bool mpfp_search, bool smbios_search)
{
    for (size_t i = 0; i < length; region += 16, i += 16) {
        if (rsdp_search == false &&
            mpfp_search == false &&
            smbios_search == false)
            return;

        if (rsdp_search == true &&
            TABLE_SIGNATURE(region[0], region[1], region[2], region[3]) == RSDP_SIGNATURE_LOW &&
            TABLE_SIGNATURE(region[4], region[5], region[6], region[7]) == RSDP_SIGNATURE_HIGH &&
            checksum_table(region, sizeof (struct rsdp)) == 0) {
            const struct rsdp_extended* rsdp = (struct rsdp_extended *) region;
            rsdt_pointer.header.length = sizeof rsdt_pointer;
            rsdt_pointer.rsdt_address = rsdp->rsdt_address;

            if (rsdp->revision > 0 /* XSDT supported */ &&
                checksum_table(region, rsdp->length) == 0) {
                rsdt_pointer.xsdt_address = rsdp->xsdt_address;
            }
            rsdp_search = false;
        }

        if (mpfp_search == true &&
            TABLE_SIGNATURE(region[0], region[1], region[2], region[3]) == MPFP_SIGNATURE &&
            checksum_table(region, ((mpfp_structure *) region)->length * 0x10) == 0) {
            pc_pointer.header.length = sizeof pc_pointer;

            /* Only copy till as much space as we reserve. */
            memcpy(&pc_pointer.mpfp, region, ((mpfp_structure *) region)->length * 0x10 > sizeof (mpfp_structure) ?
                                             sizeof (mpfp_structure) : ((mpfp_structure *) region)->length * 0x10);
            mpfp_search = false;
        }

        if (smbios_search == true &&
            TABLE_SIGNATURE(region[0], region[1], region[2], region[3]) == SMBIOS_ENTRY_POINT_SIGNATURE &&
            checksum_table(region, ((struct smbios_entry_point *) region)->length) == 0) {

            pc_pointer.header.length = sizeof pc_pointer;
            pc_pointer.smbios_entry_point_address = (uint32_t) region;
            smbios_search = false;
        }
    }
}

/*! Find all the available tables. The respective gd_* tables have non-zero 
 *  size if any tables they describe have been located. */
void tables_init()
{
    /*! ACPI:
     *      first kilobyte of EBDA.
     *      BIOS ROM address space, 0xE0000 to 0xFFFFF.
     *
     *  MPFP:
     *      first kilobyte of EBDA.
     *      last kilobyte of system base memory.
     *      BIOS ROM address space, 0xE0000 to 0xFFFFF.
     *
     *  SMBIOS:
     *      BIOS ROM address space, 0xF0000 to 0xFFFFF.
     */

    /* The BDA is supposed to contain a pointer to the EBDA. */
    uint32_t ebda = *((uint16_t*) 0x40E) << 4;
    uint32_t base_memory = get_base_memory();

    if (ebda && ebda > base_memory && ebda < 0xA0000) {
        search_region((uint8_t*) ebda, 1024, true, true, false);
    } else {
        /*! Assume EBDA to start at end of base memory. */
        search_region((uint8_t*) base_memory, 1024, true, true, false);
    }

    /* Search in last kilobyte of system base memory. */
    search_region((uint8_t*) (base_memory - 1024), 1024, false, true && !pc_pointer.mpfp.length, false);

    /* Search in the BIOS ROM address space. */
    search_region((uint8_t*) 0xE0000, 0x10000, true && !rsdt_pointer.header.length,
                                               true && !pc_pointer.mpfp.length,
                                               false);

    /* Search in the BIOS ROM address space. */
    search_region((uint8_t*) 0xF0000, 0x10000, true && !rsdt_pointer.header.length,
                                               true && !pc_pointer.mpfp.length,
                                               true);
}

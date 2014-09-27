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

#include <gd_common.h>
#include <bal/bios_services.h>
#include <bal/mmap.h>
#include <stdbool.h>
#include <stdio.h>

/*! Type conversion from ACPI to gd* types. */
gd_memory_type acpi_to_gd[] = {
    gd_unusable_memory,      /* unused */
    gd_conventional_memory,  /* address_range_memory */
    gd_unusable_memory,      /* address_range_reserved */
    gd_acpi_reclaim_memory,  /* address_range_reclaimable */
    gd_acpi_memory_nvs,      /* address_range_nvs */
    gd_unusable_memory,      /* address_range_unusable */
    gd_unusable_memory       /* address_range_disabled */
};

gd_memory_map_table *mmap = (gd_memory_map_table*)MMAP_RESERVED_SPACE;

/*! Adds entry to memory map after checking limits. */
static void mmap_add_entry_wrapper(gd_memory_map_table *table,
                                   gd_memory_map_entry entry)
{
    if (mmap_get_size(table) < MMAP_MAX_ENTRIES)
        mmap_add_entry(table, entry);
    else
        for(;;);    /* TODO: error out */
}

static void add_acpi_range(struct address_range range)
{
    if (!(range.attributes & entry_present))
        return;

    gd_memory_map_entry entry;
    entry.physical_start = range.physical_start;
    entry.virtual_start = 0;
    entry.size = range.size;
    entry.attributes = range.attributes;

    if (range.type < (sizeof acpi_to_gd / sizeof (gd_memory_type)))
        entry.type = acpi_to_gd[range.type];
    else
        entry.type = gd_unusable_memory;

    mmap_add_entry_wrapper(mmap, entry);
}

/*! Returns true on success, otherwise false. */
static bool try_e820()
{
    struct bios_registers regs = { 
        .eax = 0x0000E820,
        .ebx = 0x00000000,  /* continuation value */
        .ecx = 24,          /* size of output buffer */
        .edx = 0x534D4150   /* SMAP */
    };
   volatile struct address_range range = { .attributes = entry_present };

    regs.es = ((uint32_t)&range & 0xFFFF0) >> 4;
    regs.edi = ((uint32_t)&range & 0x000F);
    bios_int_call(0x15, &regs);

    // Carry flag after first call means unsupported.
    if ((regs.eflags & carry_flag) || 
        (regs.eax != 0x534D4150)  ||
        (regs.ecx < 20))
        return false;

    add_acpi_range(range);

    do {
        regs.eax = 0x0000E820; regs.ecx = 24; regs.edx = 0x534D4150;
        regs.es = ((uint32_t)&range & 0xFFFF0) >> 4;
        regs.edi = ((uint32_t)&range & 0x000F);

        range.attributes = entry_present;
        bios_int_call(0x15, &regs);

        // List finished.
        if (regs.eflags & carry_flag)
            return true;

        // If invalid entry, reset.
        if ((regs.eax != 0x534D4150) || (regs.ecx < 20)) {
            mmap->header.length = sizeof (gd_table);
            return false;
        }

        add_acpi_range(range);
    } while (regs.ebx);

    return true;
}

void mmap_display()
{
    for (size_t i = 0; i < mmap_get_size(mmap); i++) {
        printf("Entry %d: %llx -> %llx, %d\n", i, mmap->entries[i].physical_start,
               mmap->entries[i].physical_start + mmap->entries[i].size,
               mmap->entries[i].type);
    }
}

void mmap_init()
{
    mmap->header.id = GD_MEMORY_MAP_TABLE_ID;
    mmap->header.length = sizeof (gd_table);

    if (!try_e820()) {
        /* fallback */
    }
}

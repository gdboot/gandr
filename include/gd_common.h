/* Copyright © 2014, Owen Shepherd & Shikhin Sethi
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef GD_COMMON_H
#define GD_COMMON_H
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

/*! A Gandr table header.
 *
 * All tables are aligned to 8 byte boundaries. When processing multiple
 * sequential tables, e.g. the boot information directory, the next table is
 * found at the address
 *      ((const char*)&table) + (table.length + 7 & ~7)
 *
 * You should verify that the size of each table is <b>at least</b> that
 * specified by the specification. You <b>must</b> accept longer tables,
 * ignoring their extra content, to facilitate future specification revisions.
 *
 * Any table with an identifier starting with a lower case letter is boot
 * services only and a kernel loader can discard it when preparing the data
 * to pass to the kernel.
 */
typedef struct {
    /*! Identifier of the table */
    uint32_t id;

    /*! Length of the table, in bytes, including this header */
    uint32_t length;
} __attribute__((packed)) gd_table;

#define GD_TABLE_ID(a, b, c, d) \
    ((uint32_t) ((a << 24) | (b << 16) | (c << 8) | (d)))

/*! Copies the table ID to the passed buffer, in "natural reading order"
 *  The passed buffer <b>must</b> at least 5 bytes in size, as this function
 *  null terminates it
 */
static inline void gd_table_id_to_string(char *buf, const gd_table *table)
{
    buf[0] = (char) (table->id >> 24 & 0xFF);
    buf[1] = (char) (table->id >> 16 & 0xFF);
    buf[2] = (char) (table->id >> 8  & 0xFF);
    buf[3] = (char) (table->id       & 0xFF);
    buf[4] = 0;
}

/*! Returns a pointer to the next table */
static inline gd_table *gd_next_table(const gd_table *table)
{
    return (gd_table*) (((char*) table) + ((table->length + 7) & ~7));
}

/*! Defined types of a memory region.
 *
 *  Type codes 0x0000_0000 - 0x7FFF_FFFF are taken from the UEFI specification.
 *  Type codes 0x8000_0000 - 0xFFFF_FFFF are Gandr defined
 */
typedef enum {
    gd_reserved_memory_type = 0,
    gd_loader_code,
    gd_loader_data,
    gd_boot_services_code,
    gd_boot_services_data,
    gd_runtime_services_code,
    gd_runtime_services_data,
    gd_conventional_memory,
    gd_unusable_memory,
    gd_acpi_reclaim_memory,
    gd_acpi_memory_nvs,
    gd_mmio,
    gd_mmio_port_space,
    gd_pal_code,

    /*! ARM EABI and similar make enum sizes dependent upon their max value
     *  Force this enum to 32-bits
     */
    gd_memory_force_size = INT_MAX
} gd_memory_type;

/*! A memory map entry. This format is aligned with, but not the same as that
 *  used by the UEFI specification. In particular, UEFI uses a pageCount member;
 *  Gandr uses a size member;
 */
typedef struct {
    /*! The type of the memory region. */
    gd_memory_type type;

    /*! Physical address at which the region begins */
    uint64_t physical_start;

    /*! Virtual address at which the loader mapped this region, if it was mapped */
    uint64_t virtual_start;

    /*! Size, in bytes, of the region */
    uint64_t size;

    /*! Attribute bits
     *  \see gd_memory_map_attribute
     */
    uint64_t attributes;
} __attribute__((packed)) gd_memory_map_entry;

/*! System memory map
 */
typedef struct {
    gd_table header;
    gd_memory_map_entry entries[];
} __attribute__((packed)) gd_memory_map_table;
#define GD_MEMORY_MAP_TABLE_ID GD_TABLE_ID('M', 'M', 'A', 'P')

static inline size_t gd_mmap_get_size(gd_memory_map_table *table)
{
    return (table->header.length - sizeof (gd_table)) / sizeof (gd_memory_map_entry);
}

/*! ACPI Root System Description Table Pointer.
 *
 *  On ACPI systems, Gandr will retrieve the RSDT Pointer from the system
 *  firmware and place it in this table.
 *
 *  These addresses are always physical addresses (??? even EFI)
 */
typedef struct {
    gd_table header;
    /*! If the system is ACPI 2.0 or later compliant, the system XSDT address
     *  will be placed here. Otherwise, this will be zero */
    uint64_t xsdt_address;
    /*! The ACPI 1.0 RSDT address will be placed here */
    uint32_t rsdt_address;
} __attribute__((packed)) gd_rsdt_pointer_table;
#define GD_RSDT_POINTER_TABLE_ID GD_TABLE_ID('R', 'S', 'D', 'T')

typedef struct {
    uint8_t signature[4];
    uint32_t mp_config_table;
    uint8_t length;
    uint8_t version;
    uint8_t checksum;
    uint8_t feature_bytes[5];
} __attribute__((packed)) mpfp_structure;

/*! Pointers to description tables common to IBM-PC compatibles.
 *
 *  This table does not point to tables that are common to other platforms.
 */
typedef struct {
    gd_table header;

    /*! The MultiProcessor Floating Point structure. The structure can
        be present in system base memory, so if it is found, it is copied
        here. All fields null if not found. */
    mpfp_structure mpfp;

    /*! Pointer to the SMBIOS entry point table. Zero if not found. */
    uint32_t smbios_entry_point_address;
} __attribute__((packed)) gd_pc_pointer_table;
#define GD_PC_POINTER_TABLE_ID GD_TABLE_ID('P', 'C', 'T', 'B')

#endif

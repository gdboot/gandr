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

#ifndef GD_BAL_BIOS_TABLES_H
#define GD_BAL_BIOS_TABLES_H

#include <gd_common.h>
#include <stdint.h>

#define TABLE_SIGNATURE(a, b, c, d) \
    ((uint32_t) ((a << 24) | (b << 16) | (c << 8) | (d)))

struct rsdp {
    uint8_t signature[8];
    uint8_t checksum;
    uint8_t oemid[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__((packed));

struct rsdp_extended {
    /*! ACPI 2.0 and above extended version. */
    uint8_t signature[8];
    uint8_t checksum;
    uint8_t oemid[6];
    uint8_t revision;
    uint32_t rsdt_address;

    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
} __attribute__((packed));

#define RSDP_SIGNATURE_LOW              TABLE_SIGNATURE('R', 'S', 'D', ' ')
#define RSDP_SIGNATURE_HIGH             TABLE_SIGNATURE('P', 'T', 'R', ' ')

struct smbios_entry_point {
    uint8_t signature[4];
    uint8_t checksum;
    uint8_t length;
    uint8_t major_version;
    uint8_t minor_version;

    uint16_t max_struct_size;
    uint8_t entry_point_revision;
    uint8_t formatted_area[5];

    uint8_t signature_dmi[5];
    uint8_t checksum_dmi;
    uint16_t table_length;
    uint32_t table_address;
    uint16_t number_of_structs;
    uint8_t bcd_revision;
} __attribute__((packed));

#define SMBIOS_ENTRY_POINT_SIGNATURE    TABLE_SIGNATURE('_', 'S', 'M', '_')
#define MPFP_SIGNATURE                  TABLE_SIGNATURE('_', 'M', 'P', '_')

/*! Find all the available tables. The respective gd_* tables have non-zero 
 *  size if any tables they describe have been located. */
void tables_init();

#endif

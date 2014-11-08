/* Copyright © 2014, Shikhin Sethi
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

#ifndef GD_BAL_BIOS_MMAP_H
#define GD_BAL_BIOS_MMAP_H

#include <stdint.h>
#include <limits.h>

/*! The address range types defined by ACPI. */
typedef enum {
    address_range_memory = 1,
    address_range_reserved,
    address_range_reclaimable,
    address_range_nvs,
    address_range_unusable,
    address_range_disabled,

    acpi_memory_force_size = INT_MAX
} acpi_memory_type;

/* The attributes defined by ACPI. */
typedef enum {
    entry_present = (1 << 0),
    address_range_non_volatile = (1 << 1),
    address_range_slow_access = (1 << 2),
    address_range_error_log = (1 << 3),

    acpi_attributes_force_size = INT_MAX
} acpi_attributes;

struct address_range {
    uint64_t physical_start;
    uint64_t size;
    acpi_memory_type type;
    acpi_attributes attributes;
} __attribute__((packed));

/*! Get the amount of base memory, as reported by int 0x12. */
uint32_t get_base_memory();

void mmap_init();

#endif

/* Copyright © 2014, Shikhin Sethi.
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

#ifndef GD_BAL_VBE_H
#define GD_BAL_VBE_H
#include <gd_bal.h>

struct vbe_info_block {
    uint8_t signature[4];
    uint16_t version;
    uint32_t oem_string_ptr;
    uint8_t capabilities[4];
    uint32_t video_modes_ptr;
    uint16_t total_memory;

    // Added with VBE 2.0+.
    uint16_t oem_software_rev;
    uint32_t oem_vendor_name_ptr;
    uint32_t oem_product_name_ptr;
    uint32_t oem_product_rev_ptr;

    uint8_t reserved[222];
    uint8_t oem_data[256];
} __attribute__((packed));

#endif

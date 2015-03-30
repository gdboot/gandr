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

struct vbe_mode_info {
    uint16_t attributes;
    uint8_t window_a_attr;
    uint8_t window_b_attr;
    uint16_t window_granularity;    /* In KiB. */
    uint16_t window_size;           /* In KiB. */
    uint16_t window_a_seg;
    uint16_t window_b_seg;
    uint32_t window_position_func;  /* Equivalent to AX=4F05h, int 0x10. */
    uint16_t bytes_per_scanline;

    /* Optional for VESA modes in v1.0/1.1. */
    uint16_t width;                 /* In pixels (graphics) or characters (text). */
    uint16_t height;                /* In pixels (graphics) or characters (text). */
    uint8_t width_of_char_cell;     /* In pixels. */
    uint8_t height_of_char_cell;    /* In pixels. */
    uint8_t memory_planes;
    uint8_t bpp;
    uint8_t banks;
    uint8_t memory_model;
    uint8_t size_of_bank;           /* In KiB. */
    uint8_t image_pages;
    uint8_t reserved_0;             /* 0x00 for VBE 1.0-2.0, 0x01 for VBE 3.0. */

    /* VBE v1.2+. */
    uint8_t red_mask_size;
    uint8_t red_field_pos;
    uint8_t green_mask_size;
    uint8_t green_field_pos;
    uint8_t blue_mask_size;
    uint8_t blue_field_pos;
    uint8_t reserved_mask_size;
    uint8_t reserved_field_position;
    uint8_t direct_color_attr;

    /* VBE 2.0+. */
    uint32_t lfb_address;
    uint32_t reserved_1;
    uint16_t reserved_2;

    /* VBE 3.0+. */
    uint16_t lin_bytes_per_scanline;
    uint8_t banked_num_images;
    uint8_t lin_num_images;
    uint8_t lin_red_mask_size;
    uint8_t lin_red_field_pos;
    uint8_t lin_green_mask_size;
    uint8_t lin_green_field_pos;
    uint8_t lin_blue_mask_size;
    uint8_t lin_blue_field_pos;
    uint8_t lin_reserved_mask_size;
    uint8_t lin_reserved_field_pos;
    uint32_t max_pixel_clock;

    uint8_t reserved_3[190];
} __attribute__((packed));

enum vbe_mode_attributes {
    VBE_MODE_ATTR_SUPPORTED                      = (1 << 0),
    VBE_MODE_ATTR_EXTENDED_INFORMATION_AVAILABLE = (1 << 1),
    VBE_MODE_ATTR_TTY_BIOS_SUPPORT               = (1 << 2),
    VBE_MODE_ATTR_COLOR_MODE                     = (1 << 3),
    VBE_MODE_ATTR_GRAPHICS_MODE                  = (1 << 4),
    VBE_MODE_ATTR_NOT_VGA_COMPATIBLE             = (1 << 5),
    VBE_MODE_ATTR_NO_VGA_COMPATIBLE_WINDOW       = (1 << 6),
    VBE_MODE_ATTR_LINEAR_FRAME_BUFFER_MODE       = (1 << 7),
    VBE_MODE_ATTR_DOUBLE_SCAN_MODE               = (1 << 8),
    VBE_MODE_ATTR_INTERLACE_MODE                 = (1 << 9),
    VBE_MODE_ATTR_HARDWARE_TRIPLE_BUFFER         = (1 << 10),
    VBE_MODE_ATTR_HARDWARE_STEREOSCOPIC_DISPLAY  = (1 << 11),
    VBE_MODE_ATTR_DUAL_DISPLAY_START_ADDRESS     = (1 << 12)
};

enum vbe_window_attributes {
    VBE_WINDOW_ATTR_RELOCATABLE = (1 << 1),
    VBE_WINDOW_ATTR_READABLE    = (1 << 2),
    VBE_WINDOW_ATTR_WRITEABLE   = (1 << 3)
};

enum vbe_memory_model {
    TEXT_MODE = 0,
    CGA_GRAPHICS,
    HERCULES_GRAPHICS,
    PLANAR,
    PACKED_PIXEL,
    NON_CHAIN_256_COLOR,
    DIRECT_COLOR,
    YUV
};

void vbe_init();

#endif

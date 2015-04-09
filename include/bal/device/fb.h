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

#ifndef GD_BAL_FB_H
#define GD_BAL_FB_H

#include <gd_bal.h>
#include <stdint.h>

struct mode_info {
    uint32_t mode_identifier;
    uint16_t mode_type;
    uint32_t mode_attributes;

    uint16_t width;                 /* In pixels (graphics) or characters (text). */
    uint16_t height;                /* In pixels (graphics) or characters (text). */
    uint8_t width_of_char_cell;     /* In pixels. */
    uint8_t height_of_char_cell;    /* In pixels. */
    uint8_t depth;
    uint32_t bytes_per_scanline;

    uint8_t red_mask_size;
    uint8_t red_field_pos;
    uint8_t green_mask_size;
    uint8_t green_field_pos;
    uint8_t blue_mask_size;
    uint8_t blue_field_pos;
    uint8_t reserved_mask_size;
    uint8_t reserved_field_pos;

    uint64_t lfb_address;
    uint32_t max_pixel_clock;
};

enum mode_attributes {
    ATTR_RESERVED_FIELD_USABLE       = (1 << 0),
    ATTR_DOUBLE_SCAN_MODE_SUPPORTED  = (1 << 1),
    ATTR_INTERLACE_MODE_SUPPORTED    = (1 << 2),
    ATTR_VGA_COMPATIBLE              = (1 << 3)
};

enum mode_type {
    DIRECT_COLOR_MODE           = 0,
    PACKED_PIXEL_MODE           = 1,
    VGA_TEXT_MODE               = 2
};

struct pixel_color {
    uint8_t red, green, blue;
    uint32_t pixel;
};

struct fb_dev {
    struct gd_device dev;
    struct mode_info cur_mode;

    uint32_t max_width, max_height;
    uint32_t cur_x, cur_y;

    uint8_t *back_buffer, *front_buffer;
    uint32_t back_buffer_start; /* treat back buffer as circular buffer */

    struct pixel_color foreground, background;

    void (*fb_write_char_depth)(struct fb_dev*, char c);
};

void fb_init(struct fb_dev *dev, struct mode_info mode);

#endif
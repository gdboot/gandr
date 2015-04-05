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

#include <bal/bios_services.h>
#include <bal/vbe.h>
#include <gd_queue.h>
#include <stdlib.h>
#include <string.h>

volatile struct vbe_info_block vbe_info_block;

typedef struct mode_list_entry {
    SLIST_ENTRY(mode_list_entry)        node;
    struct mode_info                    entry;
} mode_list_entry;

static SLIST_HEAD(mode_list, mode_list_entry) modes = SLIST_HEAD_INITIALIZER(&modes);

struct mode_brief {
    uint16_t width;                 /* In pixels (graphics) or characters (text). */
    uint16_t height;                /* In pixels (graphics) or characters (text). */
    uint8_t bpp;
    uint8_t text;
} vesa_defined_modes[] = {
    {640, 400, 8, 0},
    {640, 480, 8, 0},
    {800, 600, 4, 0},
    {800, 600, 8, 0},
    {1024, 768, 4, 0},
    {1024, 768, 8, 0},
    {1280, 1024, 4, 0},
    {1280, 1024, 8, 0},
    {80, 60, 0, 1},
    {132, 25, 0, 1},
    {132, 43, 0, 1},
    {132, 50, 0, 1},
    {132, 60, 0, 1},
    {320, 200, 15, 0},
    {320, 200, 16, 0},
    {320, 200, 24, 0},
    {640, 480, 15, 0},
    {640, 480, 16, 0},
    {640, 480, 24, 0},
    {800, 600, 15, 0},
    {800, 600, 16, 0},
    {800, 600, 24, 0},
    {1024, 768, 15, 0},
    {1024, 768, 16, 0},
    {1024, 768, 24, 0},
    {1280, 1024, 15, 0},
    {1280, 1024, 16, 0},
    {1280, 1024, 24, 0}
};

mode_list_entry vga_modes[] = {
    {
        /* 80x25 text mode */
        .entry = {
            .mode_identifier = 0x03,
            .mode_type = VGA_TEXT_MODE,

            .width = 80,
            .height = 25,
            .width_of_char_cell = 9,
            .height_of_char_cell = 16,
            .depth = 0,
            .bytes_per_scanline = 80 * 2,

            .lfb_address = 0xB8000
        }
    },
};

void vbe_init()
{
    // Check for valid video card.
    struct bios_registers regs = { .eax = 0x1A00 };
    bios_int_call(0x10, &regs);

    // Not supported.
    if ((regs.eax & 0xFF) != 0x1A)
        return;

    // Add VGA modes to the mode list.
    SLIST_INSERT_HEAD(&modes, &vga_modes[0], node);

    regs.eax = 0x4F00;
    regs.es = rm_seg_from_ptr((const void*)&vbe_info_block);
    regs.edi = rm_offset_from_ptr((const void*)&vbe_info_block);

    vbe_info_block.signature[0] = 'V';
    vbe_info_block.signature[1] = 'B';
    vbe_info_block.signature[2] = 'E';
    vbe_info_block.signature[3] = '2';

    bios_int_call(0x10, &regs);

    if ((regs.eax & 0xFF) != 0x4F || ((regs.eax >> 8) & 0xFF))
        return;

    if (vbe_info_block.signature[0] != 'V' ||
        vbe_info_block.signature[1] != 'E' ||
        vbe_info_block.signature[2] != 'S' ||
        vbe_info_block.signature[3] != 'A')
        return;

    // Add VBE modes to the mode list.
    for (uint16_t *mode_identifiers = rm_ptr_from_far(vbe_info_block.video_modes_ptr);
         *mode_identifiers != 0xFFFF; mode_identifiers++) {
        // Get mode information.
        regs.eax = 0x4F01; regs.ecx = *mode_identifiers;
        memset(&scratch_buffer, 0, sizeof (struct vbe_mode_info));
        regs.es = rm_seg_from_ptr((const void*)&scratch_buffer);
        regs.edi = rm_offset_from_ptr((const void*)&scratch_buffer);

        bios_int_call(0x10, &regs);

        if ((regs.eax & 0xFF) != 0x4F) /* Function not supported. */
            return;
        else if ((regs.eax >> 8) & 0xFF) /* Unsuccessful. */
            continue;

        struct vbe_mode_info* vbe_mode_info = (struct vbe_mode_info*)&scratch_buffer;
        struct mode_info new_mode; memset(&new_mode, 0, sizeof new_mode);
        if (!(vbe_mode_info->attributes & VBE_MODE_ATTR_SUPPORTED))
            continue;

        // TODO: support monochrome in the future.
        if (!(vbe_mode_info->attributes & VBE_MODE_ATTR_COLOR_MODE))
            continue;

        new_mode.mode_identifier = *mode_identifiers;
        if (vbe_mode_info->attributes & VBE_MODE_ATTR_DOUBLE_SCAN_MODE)
            new_mode.mode_attributes |= ATTR_DOUBLE_SCAN_MODE_SUPPORTED;
        if (vbe_mode_info->attributes & VBE_MODE_ATTR_INTERLACE_MODE)
            new_mode.mode_attributes |= ATTR_INTERLACE_MODE_SUPPORTED;
        if (vbe_mode_info->direct_color_attr & VBE_DC_ATTR_RESERVED_FIELD_USABLE)
            new_mode.mode_attributes |= ATTR_RESERVED_FIELD_USABLE;

        new_mode.mode_attributes |= ATTR_VGA_COMPATIBLE;
        new_mode.bytes_per_scanline = vbe_mode_info->bytes_per_scanline;
        if (vbe_info_block.version < 0x0102 &&
            !(vbe_mode_info->attributes & VBE_MODE_ATTR_EXTENDED_INFORMATION_AVAILABLE)) {
            // Use pre-defined modes.
            if (*mode_identifiers < 0x100 || *mode_identifiers > 0x11B)
                continue;

            size_t idx = *mode_identifiers - 0x100;
            if (vesa_defined_modes[idx].bpp == 4) continue; /* no planar modes supported */

            new_mode.mode_type = vesa_defined_modes[idx].text ? VGA_TEXT_MODE : PACKED_PIXEL_MODE;
            new_mode.width = vesa_defined_modes[idx].width;
            new_mode.height = vesa_defined_modes[idx].height;
            new_mode.depth = vesa_defined_modes[idx].bpp;
        } else {
            if (vbe_mode_info->memory_planes != 1) continue;
            if (vbe_mode_info->memory_model != VBE_MM_TEXT_MODE &&
                vbe_mode_info->memory_model != VBE_MM_PACKED_PIXEL &&
                vbe_mode_info->memory_model != VBE_MM_DIRECT_COLOR)
                continue;
            if (vbe_mode_info->memory_model != VBE_MM_TEXT_MODE &&
                vbe_mode_info->bpp != 8 &&
                vbe_mode_info->bpp != 15 &&
                vbe_mode_info->bpp != 16 &&
                vbe_mode_info->bpp != 24 &&
                vbe_mode_info->bpp != 32)
                continue;

            new_mode.width = vbe_mode_info->width;
            new_mode.height = vbe_mode_info->height;
            new_mode.depth = vbe_mode_info->bpp;
            switch (vbe_mode_info->memory_model) {
                case VBE_MM_TEXT_MODE:
                    new_mode.mode_type = VGA_TEXT_MODE;
                    new_mode.depth = 0;
                    new_mode.width_of_char_cell = vbe_mode_info->width_of_char_cell;
                    new_mode.height_of_char_cell = vbe_mode_info->height_of_char_cell;
                    break;
                case VBE_MM_PACKED_PIXEL:
                    new_mode.mode_type = PACKED_PIXEL_MODE;
                    break;
                case VBE_MM_DIRECT_COLOR:
                    new_mode.mode_type = DIRECT_COLOR_MODE;
            }

            if (vbe_mode_info->attributes & VBE_MODE_ATTR_NOT_VGA_COMPATIBLE) {
                // Not VGA compatible.
                new_mode.mode_attributes &= ~ATTR_VGA_COMPATIBLE;

                // If 8-bpp packed pixel, need color ramp programmable.
                if (new_mode.depth == 8 && new_mode.mode_type == PACKED_PIXEL_MODE &&
                    !(vbe_mode_info->direct_color_attr & VBE_DC_ATTR_COLOR_RAMP_PROGRAMMABLE))
                    continue;
            }

            if (vbe_info_block.version >= 0x0300)
                new_mode.max_pixel_clock = vbe_mode_info->max_pixel_clock;
        }

        if (new_mode.mode_type == DIRECT_COLOR_MODE) {
            new_mode.red_mask_size = vbe_mode_info->red_mask_size;
            new_mode.red_field_pos = vbe_mode_info->red_field_pos;
            new_mode.green_mask_size = vbe_mode_info->green_mask_size;
            new_mode.green_field_pos = vbe_mode_info->green_field_pos;
            new_mode.blue_mask_size = vbe_mode_info->blue_mask_size;
            new_mode.blue_field_pos = vbe_mode_info->blue_field_pos;
            new_mode.reserved_mask_size = vbe_mode_info->reserved_mask_size;
            new_mode.reserved_field_pos = vbe_mode_info->reserved_field_pos;
        } else if (new_mode.mode_type == PACKED_PIXEL_MODE) {
            switch (new_mode.depth) {
                case 8:
                    new_mode.red_mask_size = 3; new_mode.red_field_pos = 5;
                    new_mode.green_mask_size = 3; new_mode.green_field_pos = 2;
                    new_mode.blue_mask_size = 2; new_mode.blue_field_pos = 0;
                    new_mode.reserved_mask_size = 0; new_mode.reserved_field_pos = 15;
                    break;
                case 15:
                    new_mode.red_mask_size = 5; new_mode.red_field_pos = 10;
                    new_mode.green_mask_size = 5; new_mode.green_field_pos = 5;
                    new_mode.blue_mask_size = 5; new_mode.blue_field_pos = 0;
                    new_mode.reserved_mask_size = 1; new_mode.reserved_field_pos = 15;
                    break;
                case 16:
                    new_mode.red_mask_size = 5; new_mode.red_field_pos = 11;
                    new_mode.green_mask_size = 6; new_mode.green_field_pos = 5;
                    new_mode.blue_mask_size = 5; new_mode.blue_field_pos = 0;
                    new_mode.reserved_mask_size = 0; new_mode.reserved_field_pos = 0;
                    break;
                case 24:
                    new_mode.red_mask_size = 8; new_mode.red_field_pos = 16;
                    new_mode.green_mask_size = 8; new_mode.green_field_pos = 8;
                    new_mode.blue_mask_size = 8; new_mode.blue_field_pos = 0;
                    new_mode.reserved_mask_size = 0; new_mode.reserved_field_pos = 0;
                    break;
                case 32:
                    new_mode.red_mask_size = 8; new_mode.red_field_pos = 16;
                    new_mode.green_mask_size = 8; new_mode.green_field_pos = 8;
                    new_mode.blue_mask_size = 8; new_mode.blue_field_pos = 0;
                    new_mode.reserved_mask_size = 8; new_mode.reserved_field_pos = 24;
                    break;
            }
        }

        if (vbe_info_block.version >= 0x0200 &&
            (vbe_mode_info->attributes & VBE_MODE_ATTR_LINEAR_FRAME_BUFFER_MODE)) {
            new_mode.lfb_address = vbe_mode_info->lfb_address;
            if (vbe_info_block.version >= 0x0300) {
                new_mode.bytes_per_scanline = vbe_mode_info->lin_bytes_per_scanline;
                if (new_mode.mode_type == DIRECT_COLOR_MODE) {
                    new_mode.red_mask_size = vbe_mode_info->lin_red_mask_size;
                    new_mode.red_field_pos = vbe_mode_info->lin_red_field_pos;
                    new_mode.green_mask_size = vbe_mode_info->lin_green_mask_size;
                    new_mode.green_field_pos = vbe_mode_info->lin_green_field_pos;
                    new_mode.blue_mask_size = vbe_mode_info->lin_blue_mask_size;
                    new_mode.blue_field_pos = vbe_mode_info->lin_blue_field_pos;
                    new_mode.reserved_mask_size = vbe_mode_info->lin_reserved_mask_size;
                    new_mode.reserved_field_pos = vbe_mode_info->lin_reserved_field_pos;
                }
            }
        } else {
            if (new_mode.height *
                new_mode.bytes_per_scanline > vbe_mode_info->window_size)
                continue; /* can fit in one window? */
            if ((vbe_mode_info->window_a_attr ==
                (VBE_WINDOW_ATTR_SUPPORTED | VBE_WINDOW_ATTR_READABLE |
                 VBE_WINDOW_ATTR_WRITEABLE)) && vbe_mode_info->window_a_seg)
                new_mode.lfb_address = vbe_mode_info->window_a_seg << 4;
            else if ((vbe_mode_info->window_b_attr ==
                (VBE_WINDOW_ATTR_SUPPORTED | VBE_WINDOW_ATTR_READABLE |
                 VBE_WINDOW_ATTR_WRITEABLE)) && vbe_mode_info->window_b_seg)
                new_mode.lfb_address = vbe_mode_info->window_b_seg << 4;
            else if (vbe_mode_info->window_a_seg &&
                     !(vbe_mode_info->window_a_attr & VBE_WINDOW_ATTR_SUPPORTED))
                new_mode.lfb_address = vbe_mode_info->window_a_seg << 4;
            else if (vbe_mode_info->window_b_seg &&
                     !(vbe_mode_info->window_b_attr & VBE_WINDOW_ATTR_SUPPORTED))
                new_mode.lfb_address = vbe_mode_info->window_b_seg << 4;
            else continue;
        }

        mode_list_entry *new_entry = malloc(sizeof *new_entry);
        if (!new_entry) {
            // TODO: Show warning.
            return;
        }

        // Add to list.
        memcpy(&new_entry->node, &new_mode, sizeof (struct mode_info));
        SLIST_INSERT_HEAD(&modes, new_entry, node);
    }
}
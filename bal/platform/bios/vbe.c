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

typedef struct vbe_mode_list_entry {
    SLIST_ENTRY(vbe_mode_list_entry)    node;
    uint32_t                            mode_identifier;
    struct vbe_mode_info                entry;
} vbe_mode_list_entry;

static SLIST_HEAD(vbe_mode_list, vbe_mode_list_entry) modes = SLIST_HEAD_INITIALIZER(&modes);

vbe_mode_list_entry vga_text_modes[] = {
    {
        /* 80x25 text mode */
        .mode_identifier = 0x03,
        .entry = {
            .attributes = VBE_MODE_ATTR_SUPPORTED |
                          VBE_MODE_ATTR_EXTENDED_INFORMATION_AVAILABLE |
                          VBE_MODE_ATTR_TTY_BIOS_SUPPORT,
            .window_a_attr = VBE_WINDOW_ATTR_RELOCATABLE |
                             VBE_WINDOW_ATTR_READABLE |
                             VBE_WINDOW_ATTR_WRITEABLE,
            .window_b_attr = 0,
            .window_granularity = 16,       /* In KiB. */
            .window_size = 16,              /* In KiB. */
            .window_a_seg = 0xB800,
            .window_position_func = 0,
            .bytes_per_scanline = 80 * 2,

            .width = 80,
            .height = 25,
            .width_of_char_cell = 9,        /* In pixels. */
            .height_of_char_cell = 16,      /* In pixels. */
            .memory_planes = 1,
            .bpp = 16,
            .banks = 1,
            .memory_model = TEXT_MODE,
            .size_of_bank = 0,
            .image_pages = 8 - 1,
            .reserved_0 = 1,

            .banked_num_images = 8 - 1
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

    // Add VGA text modes to the mode list.
    SLIST_INSERT_HEAD(&modes, &vga_text_modes[0], node);

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
        regs.es = rm_seg_from_ptr((const void*)&scratch_buffer);
        regs.edi = rm_offset_from_ptr((const void*)&scratch_buffer);

        bios_int_call(0x10, &regs);

        if ((regs.eax & 0xFF) != 0x4F) /* Function not supported. */
            return;
        else if ((regs.eax >> 8) & 0xFF) /* Unsuccessful. */
            continue;

        if (!(((struct vbe_mode_info*)&scratch_buffer)->attributes & VBE_MODE_ATTR_SUPPORTED))
            continue;

        vbe_mode_list_entry *new_entry = malloc(sizeof *new_entry);
        if (!new_entry) {
            // TODO: Show warning.
            return;
        }

        // Add to list.
        new_entry->mode_identifier = *mode_identifiers;
        memcpy(&new_entry->entry, &scratch_buffer, sizeof (struct vbe_mode_info));
        SLIST_INSERT_HEAD(&modes, new_entry, node);
    }
}
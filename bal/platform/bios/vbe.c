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

volatile struct vbe_info_block vbe_info_block;

void vbe_init();
void vbe_init()
{
    // Check for valid video card.
    struct bios_registers regs = { .eax = 0x1A00 };
    bios_int_call(0x10, &regs);

    // Not supported.
    if ((regs.eax & 0xFF) != 0x1A)
        return;

    // Add VGA text modes to the mode list.

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
}
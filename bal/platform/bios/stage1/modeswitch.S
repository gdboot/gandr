/* Copyright © 2013, Owen Shepherd
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

#include <bal/bios_services.h>

                .code16

                .section .data

.global gdtr
gdtr:
    .word (8 * 6) - 1
    .long gdt

gdt:
    .long 0x00000000, 0x00000000

    // CS16.
    .word 0xFFFF, 0x0000
    .byte 0x00, 0x9A, 0x0F, 0x00

    // DS16.
    .word 0xFFFF, 0x0000
    .byte 0x00, 0x92, 0x0F, 0x00

    // CS32.
    .word 0xFFFF, 0x0000
    .byte 0x00, 0x9A, 0xCF, 0x00

    // DS32.
    .word 0xFFFF, 0x0000
    .byte 0x00, 0x92, 0xCF, 0x00

    // CS64.
    .long 0x00000000
    .byte 0x00, 0x98, 0x20, 0x00

                // Bits that need to be set to enter and exit protected mode
                // If we are loading a 64-bit BAL, these will be adjusted to set/clear
                // CR0.PG as well.
                .global pm_cr0_bits, pm_cr0_mask
                .align 4
pm_cr0_bits:    .long  0x00000001
pm_cr0_mask:    .long ~0x00000001

                .section .text
                .global rm_to_pm16, pm16_to_rm

//! Switch to PM16
//! Includes reloading all segment registers
//! FS will be set to DS32
rm_to_pm16:     pushf
                cli
                push %eax

                mov %cr0, %eax
                or pm_cr0_bits, %eax
                mov %eax, %cr0

                mov $PM_DS16, %ax
                mov %ax, %ds
                mov %ax, %es
                mov %ax, %ss
                mov %ax, %gs
                mov $PM_DS32, %ax
                mov %ax, %fs

                ljmp $PM_CS16, $1f
1:              pop %eax
                popf
                ret

//! Switch from PM16 to RM
//! All segments will be reloaded with 16-bit limits
pm16_to_rm:     pushf
                cli
                push %eax
                movw $PM_DS16, %ax
                movw %ax, %ds
                movw %ax, %es
                movw %ax, %fs
                movw %ax, %gs
                movw %ax, %ss

                mov %cr0, %eax
                and pm_cr0_mask, %eax
                mov %eax, %cr0

                movw $RM_DS, %ax
                movw %ax, %ds
                movw %ax, %es
                movw %ax, %fs
                movw %ax, %gs
                movw %ax, %ss
                
                ljmp $RM_CS, $1f
1:              pop %eax
                popf
                ret

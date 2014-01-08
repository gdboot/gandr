/* Copyright © 2013-2014, Shikhin Sethi
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

/*
 * Stage 1 (disk dependent, firmware dependent) initialization code.
 */

.code16
.section base.start

#define BOOT_STACK  0x8000

/*
 * Entry point.
 *     dl -> the drive number.
 *     cs:ip -> the linear address 0x7C00.
 */
.global _start
_start:
#ifdef BD_ELTORITO
    // Have space for the boot information table passed by mkisofs.
    jmp .relocate

.align 8
.global eltorito_info
eltorito_info:
    .long 0
    .long 0
    .long 0
    .long 0
    .skip 40, 0

.relocate:
#endif
    // Relocate us down to 0x0600.
#ifdef BD_PXE
    push %es
#endif

    xor %ax, %ax
    mov %ax, %es
    mov %ax, %ds

    mov $0x0600, %di
    mov $0x7C00, %si
    
#ifdef BD_PXE
    mov 0x2000 - 0x0600, %cx
#elif defined(BD_ELTORITO)
    mov $2048, %cx
#else
    mov $512, %cx
#endif

    cld
    rep movsb

#ifdef BD_PXE
    pop %es
#endif

    // Do a far jump to reset CS to 0x0000.
    ljmp $0x0000, $reset_cs

reset_cs:
#ifndef BD_PXE
    // Set the stack just below where we start.
    // For PXE, there is already a stack set up.
    // And, storage_init requires %ss:%sp to be PXE-sane.
    mov %ax, %ss
    mov $BOOT_STACK, %sp
#endif

    call output_init
    call storage_init

#ifdef BD_PXE
    xor %ax, %ax
    mov %ax, %ss
    mov $BOOT_STACK, %esp
#endif

    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    // Enable A20 gate.
    call a20_enable

    .hlt:
        hlt
        jmp .hlt

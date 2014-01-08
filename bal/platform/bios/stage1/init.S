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

#include <bal/bios_services.h>

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

    // Invert the IOPL bits, since on 8086/80186 they are hardwired to 1.
    // In real mode on the 286, they are always 0, though.
    pushf
    pop %ax
    xor $0x30, %ah

    push %ax
    popf
    pushf
    pop %cx

    // Test if bits changed, or not.
    xor %cx, %ax
    test %ah, %ah
    jnz .error_not_386

    call output_init
    call storage_init

#ifdef BD_PXE
    xor %ax, %ax
    mov %ax, %ss
    mov $BOOT_STACK, %esp
#else
    // Clear higher word of ESP.
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

    .error_not_386:
        /* TODO: ERROR. */
        jmp .hlt


.text
Lgo_stage2:
    // Find highest point in conventional memory - that will be our new stack
    int $0x12
    movzx %ax, %eax
    shl $10, %eax

    // Stick the right segment in the jump pointer structure
    mov bal_is_64bit, %bl
    mov $PM_CS32, %cx
    test %bl, %bl
    jnz Lnot_64bit
    mov $PM_CS64, %cx
Lnot_64bit:
    mov %cx, stage2_entry+4

    // TOOD: Find entrypoint of stage2
    // That goes into the jump pointer structure :-)
    // movl (...), stage2_entry

    // Setup (64/32-bit) protected mode and go
    call rm_to_pm16
    mov PM_DS32, %cx
    mov %cx, %ds
    mov %cx, %es
    mov %cx, %fs
    mov %cx, %gs
    mov %cx, %ss
    mov %eax, %esp

    mov $api_table, %edi
    data32 ljmp *stage2_entry

    .comm stage2_entry, 6
    .comm bal_is_64bit, 1
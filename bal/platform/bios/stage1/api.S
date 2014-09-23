/* Copyright © 2013-2014, Owen Shepherd & Shikhin Sethi
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

.global api_table
.global stack_top, bal_stack_top

stack_top       = 0x8000
bal_stack_top   = 0x10000

.data
api_table:
    .ascii "BIOS"
    .long api_table_end - api_table
    .long stack_top
    .word api_far_call, PM_CS16
    .word api_int_call, PM_CS16
api_table_end:

.text
api_far_call:
    // Set registers as passed
    call pm16_to_rm
    popw %ds
    popw %es
    popw %fs
    popw %gs

    popal
    popf

    popl far_call_ptr
    lcall *far_call_ptr

    pushf
    pushal
    pushw %gs
    pushw %fs
    pushw %es
    pushw %ds
    call rm_to_pm16

    // Switch back to normal running
    mov $PM_DS32, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    movl stack_top - 4, %esp
    // Jump back into (32-bit/64-bit) code
    lretl
    .comm far_call_ptr, 4

api_int_call:
    call pm16_to_rm
    // Patch the int instruction
    mov %al, (1f+1)
    // Set registers as passed
    popw %ds
    popw %es
    popw %fs
    popw %gs

    popal
    popf

1:  int $0x00

    pushf
    pushal
    pushw %gs
    pushw %fs
    pushw %es
    pushw %ds
    call rm_to_pm16

    // Switch back to normal running
    mov $PM_DS32, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    movl stack_top - 4, %esp
    // Jump back into (32-bit/64-bit) code
    lretl

/* Copyright © 2014, Shikhin Sethi
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

.code16
.text

//! Wait till keyboard input buffer is clear.
kyb_wait_input_buf:
    in $0x64, %al
    test $2, %al    // If second bit is 0, then empty.
    jnz kyb_wait_input_buf
    ret

//! Waits till keyboard output buffer is clear.
kyb_wait_output_buf:
    in $0x64, %al
    test $1, %al    // If first bit is 0, then empty.
    jnz kyb_wait_output_buf
    ret

/*! Try to enable A20.
 *      \return Aborts boot on failure.
 */
.global a20_enable
a20_enable:
    push %ax
    pushf

    call a20_check
    jc .return

    // Try the BIOS, enable A20 gate.
    mov $0x2401, %ax
    int $0x15

    call a20_check
    jc .return

    // Try the keyboard controller.
    cli
    call kyb_wait_input_buf
    mov $0xAD, %al
    out %al, $0x64          // Disable.

    call kyb_wait_input_buf
    mov $0xD0, %al
    out %al, $0x64          // Read controller output port (bit 1 enables A20).

    call kyb_wait_output_buf
    in $0x60, %al
    push %ax                // Save controller output port.

    call kyb_wait_input_buf
    mov $0xD1, %al
    out %al, $0x64          // Write to controller output port.

    call kyb_wait_input_buf
    pop %ax
    or $2, %al
    out %al, $0x60

    call kyb_wait_input_buf
    mov $0xAE, %al
    out %al, $0x64          // Enable.

    call kyb_wait_input_buf

    call a20_check
    jc .return

    // Try fast A20.
    in $0x92, %al           // Get the value from 0x92 port. 
    test $2, %al            // If the fast A20 bit is already set, then unreliable.
    jnz .error

    or $2, %al              // Enable Fast A20.
    and $0xFE, %al          // Bit 0 is used for fast reset.
    out %al, $0x92

    call a20_check
    jnc .error

    .return:
        popf
        pop %ax
        ret

    .error:
        /* TODO: ERROR. */
        hlt
        jmp .error

/*! Checks the status of the A20 line.
 *      \return Carry set if the A20 line is enabled.
 */
a20_check:
    push %si
    push %di
    push %es

    xor %ax, %ax
    not %ax
    mov %ax, %es

    mov $0x514, %di     // ES:DI points to 0x100504.
    mov $0x504, %si     // DS:SI points to 0x504.

    movb $0, %ds:(%si)  // 0x500 now has 0x00 stored in it.
    movb %al, %es:(%di) // Write 0xFF at 0x100504 to see if it overwrites 0x504.

    lodsb
    cmp %ah, %al      // If overwritten, then A20 is not enabled.
    je .disabled

    stc
    jmp .ret

    .disabled:
        clc
    .ret:
        pop %es
        pop %di
        pop %si
        ret

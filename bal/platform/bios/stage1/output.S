/* Copyright © 2013, Shikhin Sethi
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
.section base

active_page:    .byte 0

//! Initiates output.
.global output_init
output_init:
    pushal
    push %es

    // Set mode (ah = 0x00) to 80*25 text mode (al = 0x03).
    mov $0x0003, %ax
    int $0x10

    // Point es to video memory.
    mov $0xB800, %ax
    mov %ax, %es

    // Get active page.
    xor %bx, %bx
    mov $0x0F, %ah
    int $0x10
    mov %bh, (active_page)

    // Set the cursor to 00,00.
    // ah = 0x02, int 0x10, to set cursor position.
    // bh = page number.
    // dh = row; dl = column.
    xor %dx, %dx
    mov $0x02, %ah
    int $0x10

    // Hide the cursor.
    // ah = 0x01, int 0x10, to set cursor shape.
    // cl = bottom scan line containing cursor; ch = 0x10 = invisible cursor.
    // al needs to be equal to current mode (0x03).
    mov $0x1000, %cx
    mov $0x0103, %ax
    int $0x10

    // Clear screen to black background, white foreground, spaces.
    xor %di, %di
    mov $1000, %ecx
    mov $0x0F200F20, %eax
    rep stosl

    pop %es
    popal
    ret

/*! Outputs a null terminated string.
 *      \param es:si point to null terminated string
 */
.global output_string
output_string:
    push %si
    push %ax

    // Some BIOS may destroy BP if display is getting scrolled.
    push %bp

    .loop:
        // Load the value at [es:si] in al.
        lodsb

        // Stop if it is the null terminator.
        test %al, %al
        jz .done

        cmp $0x0A, %al      // New line?
        jne 0f

        mov $0x0E, %ah
        mov (active_page), %bh
        int $0x10

        // Output carriage return.
        mov $0x0D, %al
    0:
        // Output character; ah = 0x0E; al = character; bh = page number;
        mov $0x0E, %ah
        mov (active_page), %bh
        int $0x10
        jmp .loop

    .done:
        pop %bp

        pop %ax
        pop %si
        ret

// Hexadecimal to string buffer.
// 0x30 to get ascii 0.
hexadecimal_buffer:
    .byte '0', 'x', 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30
    .byte 0x00

/*! Outputs a hexadecimal value.
 *      \param eax value to output
 */
.global output_hexadecimal
output_hexadecimal:
    pushal

    mov $8, %ecx
    // +2 for the 0x part; + 7 to reach last byte.
    mov $hexadecimal_buffer + 2 + 7, %di
    movl $0x30303030, -3(%di)
    movl $0x30303030, -7(%di)

    .loop_digits:
        // Get the lower 4 bits.
        mov %al, %bl
        and $0x0F, %bl

        // If it is greater than 9, then add 7 to get 'A' (0x41).
        cmp $9, %bl
        jbe .numeric

        add $7, %bl

    .numeric:
        add %bl, (%di)

        // Next value.
        ror $4, %eax
        dec %di
        loop .loop_digits

        mov $hexadecimal_buffer, %si
        call output_string

    popal
    ret

/* Copyright © 2013-2014, Shikhin Sethi
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

.code16
.section base

/*! Convert integer to string (base 16).
 *      \param %eax the original integer.
 *      \param %di the destination.
 */
.global itoa
itoa:
    push %ebx
    push %cx

    mov $8, %cx

    mov $0x30303030, %ebx
    movl %ebx, (%di)
    movl %ebx, 4(%di)

    add $8, %di
    .loop_digits:
        dec %di
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
        loop .loop_digits

    pop %cx
    pop %ebx
    ret

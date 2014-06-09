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

.code16
.text

/*! Converts character to uppercase.
 *      \param al character.
 *      \return al contains the uppercase character.
 */
.global toupper
toupper:
     cmp $'a', %al
     jb .converted
     cmp $'z', %al
     ja .converted

     add $('A' - 'a'), %al
     .converted:
        ret

/*! Gets the length of the string.
 *      \param ds:si source
 *      \return Length of string in ecx.
 */
.global strlen
strlen:
    push %si

    xor %ecx, %ecx
    0:
        lodsb
        test %al, %al
        jz .return

        inc %ecx
        jmp 0b

    .return:
        pop %si
        ret

/*! Copies n characters, unless source string terminates.
 *  Doesn't handle segment overflow on destination. FIX if needed.
 *      \param ds:si source
 *      \param es:di destination
 *      \param cx maximum limit
 *      \return Incremented ds:si, es:di.
 */
.global strncpy
strncpy:
    push %ax
    push %cx
    push %bx

    .loop:
        lodsb
        stosb

        test %si, %si
        jnz 0f

        // Segment overflow.
        mov %ds, %bx
        add $0x1000, %bx
        mov %bx, %ds

    0:
        test %al, %al
        jz .ret

        loop .loop

    .ret:
        pop %bx
        pop %cx
        pop %ax

        ret

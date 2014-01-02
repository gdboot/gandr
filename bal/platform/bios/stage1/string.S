/*
 * String functions.
 */

.code16
.text

/*
 * Copies n characters, unless source string terminates.
 * Doesn't handle segment overflow on destination. FIX if needed.
 *
 *     ds:si -> source.
 *     es:di -> destination.
 *     cx -> maximum limit.
 *
 * Returns:
 *     Incremented ds:si, es:di.
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

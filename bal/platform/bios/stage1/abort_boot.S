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
.section base

.global stage1_error_msg
stage1_error_msg: .ascii "Gandr stage1 fatal error. "
                  .byte 0

/*! Abort boot.
 *  Plays note A, 5th octave, 880Hz. Credits to contrapunctus
 *  from #music at freenode.
 *      \param %si points to error string.
 */
.global abort_boot
abort_boot:
    push %si
    mov $stage1_error_msg, %si
    call output_string
    pop %si
    call output_string

.global abort_boot_hcf
abort_boot_hcf:
    cli

    /* Write to command register (0x43).
     *   10XXXXXXb - channel 2.
     *   11XXXXb - access mode lobyte/hibyte.
     *   011Xb - mode 3, square wave generator.
     *   0b - 16-bit binary mode.
     */
    mov $0xB6, %al
    outb %al, $0x43

    // I/O delay.
    jmp . + 2
    jmp . + 2

    /*
     *   reloadValue = 1193180/frequency; (1193180 -> PIT oscillator)
     *               = 1193180/880 = 0x54C;
     */

    // Write to channel 2, data port (0x42).
    // Lower 8-bits.
    mov $0x4C, %al
    out %al, $0x42

    jmp . + 2
    jmp . + 2

    // Upper 8-bits.
    mov $0x05, %al
    out %al, $0x42

    // Port 0x61 controls gate input for PC speaker.
    // Bit 0 enables timer input to PC speaker; bit 1 enables speaker.
    in $0x61, %al
    or $0x03, %al 
    out %al, $0x61

    // Can enable interrupts.
    sti

    .halt:
        hlt
        jmp .halt

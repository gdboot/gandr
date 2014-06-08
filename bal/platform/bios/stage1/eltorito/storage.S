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

//! The boot information table (passed by mkisofs) structure.
    .struct 0
eltorito_info_pvd_lba:
    .struct eltorito_info_pvd_lba + 4
eltorito_info_stage1_lba:
    .struct eltorito_info_stage1_lba + 4
eltorito_info_stage1_size:
    .struct eltorito_info_stage1_size + 4
eltorito_info_checksum:

//! The LBA packet structure.
    .struct 0
lba_packet_size:
    .struct lba_packet_size + 2
lba_packet_blocks:
    .struct lba_packet_blocks + 2
lba_packet_buffer_off:
    .struct lba_packet_buffer_off + 2
lba_packet_buffer_seg:
    .struct lba_packet_buffer_seg + 2
lba_packet_lba_low:
    .struct lba_packet_lba_low + 4
lba_packet_lba_high:

.section base

boot_drive: .byte 0
lba_packet:
    .byte 0
    .byte 0
    .word 0
    .word 0
    .word 0
    .long 0
    .long 0

.equ PVD_BUFFER, 0x2000

/*! Checks if int 0x13 extensions are supported or not.
 *      \return Aborts boot on error.
 */
int13_extensions_check:
    pushal

    mov $0x41, %ah
    mov $0x55AA, %bx
    mov (boot_drive), %dl

    int $0x13
    jc .no_int13_ext

    cmp $0xAA55, %bx
    jne .no_int13_ext

    popal
    ret

    .no_int13_ext:
        /* TODO: ERROR. */
        hlt
        jmp .no_int13_ext

/*! Does a read via int 0x13 extensions.
 *      \param cx number of blocks to read
 *      \param edi destination
 *      \param ebx lba to read from
 *      \return ah and cf as set by int $0x13.
 *              ecx set to number of blocks transferred.
 */
int13_ext_read:
    push %dx
    push %si

    movb $0x10, (lba_packet + lba_packet_size) /* No 64-bit flat address */
    mov %cx, (lba_packet + lba_packet_blocks)
    mov %ebx, (lba_packet + lba_packet_lba_low)
    movl $0, (lba_packet + lba_packet_lba_high)

    mov %edi, %eax
    shr $4, %eax
    mov %ax, (lba_packet + lba_packet_buffer_seg)
    mov %di, %ax
    and $0x000F, %ax
    mov %ax, (lba_packet + lba_packet_buffer_off)

    mov $lba_packet, %si
    mov (boot_drive), %dl
    mov $0x42, %ah
    int $0x13

    movzwl (lba_packet + lba_packet_blocks), %ecx

    pop %si
    pop %dx
    ret

/*! Does a read via int 0x13 extensions.
 *      \param ecx number of blocks to read
 *      \param edi destination
 *      \param ebx lba to read from
 *      \return BIOS error code, if any, in eax.
 */
storage_read:
    xor %eax, %eax
    pushal
    push %ecx

    .multiple:
        mov (%esp), %ecx
        cmpl $0x7F, (%esp)
        jbe 0f

        // Read at most 0x7F blocks.
        mov $0x7F, %cx
    0:
        call int13_ext_read
        jc .single
        test %ah, %ah
        jnz .single

        // Successful read.
        sub %ecx, (%esp)
        add %ecx, %ebx
        shl $11, %ecx /* ecx * 2048 */
        add %ecx, %edi
        cmpl $0, (%esp)
        jne .multiple

        jmp .storage_read_ret

    .single:
        mov $1, %cx
        call int13_ext_read

        jc .storage_read_error
        test %ah, %ah
        jnz .storage_read_error

        sub %ecx, (%esp)
        add %ecx, %ebx
        shl $11, %ecx
        add %ecx, %edi
        cmpl $0, (%esp)
        jne .single

    .storage_read_ret:
        add $4, %esp
        popal
        ret

    .storage_read_error:
        movzbl %ah, %eax
        mov %eax, 32(%esp)
        jmp .storage_read_ret

/*! Initialize disk.
 *      \param dl boot drive
 */
.global storage_init
storage_init:
    pushal

    mov %dl, (boot_drive)

    // Check for int 0x13 extensions.
    call int13_extensions_check

    mov (eltorito_info + eltorito_info_stage1_size), %ecx
    cmp $0x800, %ecx
    jbe 0f

    // Read rest of boot file at 0x600 + 0x800.
    mov $(0x600 + 0x800), %edi
    mov (eltorito_info + eltorito_info_stage1_lba), %ebx
    inc %ebx

    // Round size to nearest 2KiB.
    add $0x7FF, %ecx
    shr $11, %ecx
    dec %ecx

    call storage_read
    test %eax, %eax
    jnz .read_error

    0:
    // Read PVD.
    mov $1, %ecx
    mov (eltorito_info + eltorito_info_pvd_lba), %ebx
    mov $PVD_BUFFER, %edi

    call storage_read
    test %eax, %eax
    jnz .read_error

    

    popal
    ret

    .read_error:
        /* TODO: ERROR. */
        hlt
        jmp .read_error

/*! Opens a file to be read from.
 *      \param esi points to the file name
 *      \return ecx contains size of the file.
 *              If file not found, eax non-zero. Else, aborts boot on error.
 */
.global file_open
file_open:
    xor %eax, %eax
    not %eax
    ret

/*! Reads the currently open file.
 *      \param edi where to read the file to
 *      \param ecx the number of bytes to read
 *      \return Aborts boot on error.
 */
.global file_read
file_read:
    xor %eax, %eax
    not %eax
    ret

/*! Close the currently open file.
 *      \return Aborts boot on error.
 */
.global file_close  
file_close:
    xor %eax, %eax
    not %eax
    ret

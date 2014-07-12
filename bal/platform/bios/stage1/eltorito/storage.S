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

//! The PVD structure.
    .struct 0
pvd:
    .struct pvd + 0
pvd_type_code:
    .struct pvd + 1
pvd_identifier:
    .struct pvd + 6
pvd_version:
    .struct pvd + 128
pvd_logical_block_size:
    .struct pvd + 156
pvd_root_directory_entry:

//! Directory record structure.
    .struct 0
directory_record:
    .struct directory_record + 0
directory_record_length:
    .struct directory_record + 1
directory_record_xar_length:
    .struct directory_record + 2
directory_record_lba:
    .struct directory_record + 10
directory_record_data_length:
    .struct directory_record + 25
directory_record_file_flags:
    .struct directory_record + 26
directory_record_file_unit_size:
    .struct directory_record + 27
directory_record_interleave_gap:
    .struct directory_record + 32
directory_record_file_identifier_len:
    .struct directory_record + 33
directory_record_file_identifier:

#define DIRREC_FLAG_DIR                 (1 << 1)
#define DIRREC_FLAG_XAR_RECORD          (1 << 3)
#define DIRREC_FLAG_MULTIPLE_RECORDS    (1 << 7)

.bss
boot_drive: .byte 0
lba_packet:
    .byte 0
    .byte 0
    .word 0
    .word 0
    .word 0
    .long 0
    .long 0

.equ SECTOR_SIZE, 2048
.equ PVD_BUFFER, 0x2000

.section base

/*! Converts character to uppercase.
 *      \param al character.
 *      \return al contains the uppercase character.
 */
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
 *      \return Length of string in cx.
 */
strlen:
    push %si
    push %ax

    xor %cx, %cx
    0:
        lodsb
        test %al, %al
        jz .return

        inc %cx
        jmp 0b

    .return:
        pop %ax
        pop %si
        ret

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
        mov $no_int13_ext_error_msg, %si
        jmp abort_boot

no_int13_ext_error_msg:
    .ascii "BIOS does not support int 0x13 extensions for el-torito boot."
    .byte 0

file_name:
    .word 0

/*! Print out error message.
 *      \param ah error code
 *      \param (file_name) file name (null if no file)
 */
int13_error:
    mov $stage1_error_msg, %si
    call output_string

    cmpw $0, (file_name)
    je 0f

    mov $int13_file_error_msg_1, %si
    call output_string
    mov (file_name), %si
    call output_string
    mov $int13_file_error_msg_2, %si
    call output_string

0:
    movzbl %ah, %eax
    mov $.int13_error_status, %di
    call itoa
    movzbl (boot_drive), %eax
    mov $.int13_disk, %di
    call itoa

    mov $int13_error_msg, %si
    call output_string

    jmp abort_boot_hcf

int13_file_error_msg_1:
    .ascii "File "
    .byte 0
int13_file_error_msg_2:
    .ascii ": "
    .byte 0

int13_error_msg:
    .ascii "BIOS function int 0x13 (AH = 0x42, DL = 0x"
    .int13_disk: .ascii "00000000"
    .ascii ") returned status 0x"
    .int13_error_status: .ascii "00000000"
    .ascii "."
    .byte 0

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
 *      \return Aborts boot on error.
 */
storage_read:
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

        jc int13_error
        test %ah, %ah
        jnz int13_error

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

/*! Check a directory for a particular record.
 *      \param ebx the directory's LBA
 *      \param edx length of directory data
 *      \param ds:si record identifier
 *      \return di points to the directory record.
                eax ~0 on record not found, abort for disk failure.
                Aborts boot if ISO uses XAR/multiple directory records.
 */
find_directory_record:
    xor %eax, %eax
    pushal

    call strlen
    push %cx

    .load_sector:
        mov $(PVD_BUFFER + SECTOR_SIZE), %edi
        mov $1, %ecx
        call storage_read

        .check_record:
            // Zero-length record means next-sector wrapover.
            cmpb $0, directory_record_length(%di)
            je .next_sector

            mov (%esp), %cx
            cmpb directory_record_file_identifier_len(%di), %cl
            jne .next_record

            xor %bp, %bp
            .string_cmp:
                mov (%bp, %si), %al
                call toupper
                cmp directory_record_file_identifier(%bp, %di), %al
                jne .next_record

                inc %bp
                cmp %cx, %bp
                jb .string_cmp

            // Found.
            testb $DIRREC_FLAG_XAR_RECORD, directory_record_file_flags(%di)
            jnz .xar_error
            testb $DIRREC_FLAG_MULTIPLE_RECORDS, directory_record_file_flags(%di)
            jnz .multiple_records_error

            movl %edi, 2(%esp)
            jmp .success

        .next_record:
            movzbl (%di), %ecx
            add %cx, %di

            cmp $(PVD_BUFFER + (SECTOR_SIZE * 2)), %di
            jae .next_sector

            lea (SECTOR_SIZE + PVD_BUFFER)(%edx), %ecx
            cmp %cx, %di
            jae .not_found

            jmp .check_record

    .next_sector:
        // Last sector.
        cmp $SECTOR_SIZE, %edx
        jbe .not_found

        sub $SECTOR_SIZE, %edx
        inc %ebx
        jmp .load_sector

    .success:
        add $2, %esp
        popal
        ret

    .not_found:
        add $2, %esp
        popal
        not %eax
        ret

    .xar_error:
    .multiple_records_error:
        mov $xar_multirec_error_msg, %si
        jmp abort_boot

xar_multirec_error_msg:
    .ascii "XAR or multiple records for any entry is not supported."
    .byte 0

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

    0:
    // Read PVD.
    mov $1, %ecx
    mov (eltorito_info + eltorito_info_pvd_lba), %ebx
    mov $PVD_BUFFER, %edi

    call storage_read

    cmpw $SECTOR_SIZE, (PVD_BUFFER + pvd_logical_block_size)
    jne .block_size_error

    mov $(PVD_BUFFER + pvd_root_directory_entry), %edi

    testb $DIRREC_FLAG_XAR_RECORD, directory_record_file_flags(%di)
    jnz .xar_error
    testb $DIRREC_FLAG_MULTIPLE_RECORDS, directory_record_file_flags(%di)
    jnz .multiple_records_error

    movzbl directory_record_xar_length(%di), %ebx
    add directory_record_lba(%di), %ebx
    movl directory_record_data_length(%di), %edx

    mov $boot_dir, %esi
    call find_directory_record

    cmp $0xFFFFFFFF, %eax
    je .no_boot_dir_error

    testb $DIRREC_FLAG_DIR, directory_record_file_flags(%di)
    jz .no_boot_dir_error

    movzbl directory_record_xar_length(%di), %eax
    add directory_record_lba(%di), %eax
    movl %eax, (boot_dir_lba)
    movl directory_record_data_length(%di), %eax
    movl %eax, (boot_dir_len)

    popal
    ret

    .block_size_error:
        mov $block_size_error_msg, %si
        jmp abort_boot

    .no_boot_dir_error:
        mov $no_boot_dir_error_msg, %si
        jmp abort_boot

block_size_error_msg:
    .ascii "Block size apart from 2048 bytes not supported with ISOs."
    .byte 0

no_boot_dir_error_msg:
    .ascii "Invalid ISO with no boot directory."
    .byte 0

boot_dir:   .ascii "boot"
            .byte 0

.bss
boot_dir_lba:   .long 0
boot_dir_len:   .long 0

file_dir_rec:   .long 0

.text

/*! Opens a file to be read from.
 *  NOTE: doesn't support sub-directories.
 *      \param si points to the file name
 *              If file not found, eax non-zero. Else, aborts boot on error.
 */
.global file_open
file_open:
    xor %eax, %eax
    pushal

    mov %si, (file_name)

    movl (boot_dir_lba), %ebx
    movl (boot_dir_len), %edx

    call find_directory_record

    cmp $0xFFFFFFFF, %eax
    je .file_not_found

    testb $DIRREC_FLAG_DIR, directory_record_file_flags(%di)
    jnz .file_not_found
    movl %edi, (file_dir_rec)

    .ret:
        popal
        ret

    .file_not_found:
        popal
        not %eax
        ret

/*! Reads the currently open file.
 *      \param edi where to read the file to
 *      \return Aborts boot on error.
 */
.global file_read
file_read:
    pushal

    movl (file_dir_rec), %ebp
    
    // Get the number of blocks to read.
    movl directory_record_data_length(%bp), %ecx
    add $0x7FF, %ecx
    shr $11, %ecx

    movzbl directory_record_xar_length(%bp), %ebx
    add directory_record_lba(%bp), %ebx
    call storage_read

    popal
    ret

/*! Closes the currently open file.
 */
.global file_close
file_close:
    movw $0, (file_name)
    ret

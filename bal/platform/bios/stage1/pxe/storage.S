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

#define PXE_NEW_PRESENT (1 << 0)

#define UNDI_SHUTDOWN   0x05
#define UNLOAD_STACK    0x70
#define STOP_UNDI       0x15
#define UNDI_CLEANUP    0x02

#define GET_CACHED_INFO 0x71
#define TFTP_OPEN       0x20
#define TFTP_CLOSE      0x21
#define TFTP_READ       0x22
#define TFTP_GET_FSIZE  0x25

#define PXENV_STATUS_TFTP_FILE_NOT_FOUND 0x3B

#define PACKET_SIZE     512         //!< 512 bytes is most supported.
#define UDP_PORT        (69 << 8)   //!< 69 << 8 in big endian, word.

//! The PXENV_GET_CACHED_INFO structure.
    .struct 0
cached_info_status:
    .struct cached_info_status + 2
cached_info_packet_type:
    .struct cached_info_packet_type + 2
cached_info_buffer_size:
    .struct cached_info_buffer_size + 2
cached_info_buffer_off:
    .struct cached_info_buffer_off + 2
cached_info_buffer_seg:
    .struct cached_info_buffer_seg + 2
cached_info_buffer_limit:
    .struct cached_info_buffer_limit + 2
cached_info_sip:
    .struct cached_info_sip + 4
cached_info_gip:

//! The PXENV_TFTP_OPEN structure.
    .struct 0
tftp_open_status:
    .struct tftp_open_status + 2
tftp_open_sip:
    .struct tftp_open_sip + 4
tftp_open_gip:
    .struct tftp_open_gip + 4
tftp_open_filename:
    .struct tftp_open_filename + 128
tftp_open_port:
    .struct tftp_open_port + 2
tftp_open_packet_size:

//! The PXENV_TFTP_READ structure.
    .struct 0
tftp_read_status:
    .struct tftp_read_status + 2
tftp_read_packet_num:
    .struct tftp_read_packet_num + 2
tftp_read_buffer_size:
    .struct tftp_read_buffer_size + 2
tftp_read_buffer_off:
    .struct tftp_read_buffer_off + 2
tftp_read_buffer_seg:

.data
pxe_cached_info:
    .word 0
    .word 2
    .word 0
    .word 0  // Zero implies PXE uses own buffer.
    .word 0
    .word 0

    // SIP & GIP part of actual PXENV_GET_CACHED_INFO structure.
    .long 0
    .long 0

.bss
// The PXE API pointer.
.align 4
pxe_api_addr:
    .offset:    .word 0
    .segment:   .word 0

pxe_flags:
    .byte 0

pxe_tftp_open:
    .word 0
    .long 0
    .long 0
    .skip 128, 0
    .word 0
    .word 0

pxe_tftp_read:
    .word 0
    .word 0
    .word 0
    .word 0
    .word 0

/* Open file's size. */
file_size:  .long 0

.text

/*! Call the PXE API (both before 2.1 and after).
 *      \param ds:di the address of the input buffer
 *      \param bx the "opcode"
 */
pxe_api:
    push %ecx
    push %edx
    push %esi
    push %edi

    // NOTE: 
    // The above registers are what the legacy API also used.
    // And we push it over here, so that we have no problems with the new API.
    push %ds
    push %di
    push %bx
    
    lcall *(pxe_api_addr)
    
    add $6, %sp

    pop %edi
    pop %esi
    pop %edx
    pop %ecx
    ret

/*! Initialize PXE.
 *      \param es:bx the PXENV+ structure
 *      \param ss:sp + 4 the !PXE structure (if valid)
 *      \return Trashes bp.
 */
.global storage_init
storage_init:
    // Save sp + 4.
    mov %sp, %bp
    add $4, %bp

    pushal

    // Check PXENV signature.
    cmpl $0x4E455850 /* PXEN */, %es:(%bx)
    jne .error_no_pxe

    cmpw $0x2B56 /* V+ */, %es:4(%bx)
    jne .error_no_pxe

    // Get the size of the structure (bx + 4) in ecx.
    movzbl %es:8(%bx), %ecx
    xor %al, %al
    push %bx

    // ecx contains the number of bytes left to checksum.
    // eax contains the checksum.
    .checksum:
        add %es:(%bx), %al

        inc %bx
        loop .checksum

    // Checksum should evaluate to 0.
    test %al, %al
    jnz .error_no_pxe

    pop %bx
    // If version is greater than 2.1, try !PXE.
    cmpw $0x0201, %es:6(%bx)
    jae .new_version

    .pxenv:
        mov %es:0x0A(%bx), %ecx

        mov %ecx, (pxe_api_addr)
        jmp .cached_info

    .new_version:
        // Save es and bx, in case we need to switch back to PXENV.
        push %es
        push %bx

        // Reload es:bx to point to !PXE structure.
        mov %ss:(%bp), %ax
        mov %ss:2(%bp), %bx
        mov %ax, %es

        cmpl $0x45585021 /* !PXE */, %es:(%bx)
        jne .error_new_pxe

        movzbl %es:4(%bx), %ecx
        xor %al, %al
        push %bx

        // Now ECX contains the number of bytes on which to do the checksum, and EAX would contain the result.
        .new_version_checksum:
            add %es:(%bx), %al
            inc %bx

            loop .new_version_checksum

        pop %bx

        test %al, %al
        jnz .error_new_pxe

        mov %es:0x10(%bx), %ecx

        mov %ecx, (pxe_api_addr)
        orb $PXE_NEW_PRESENT, (pxe_flags)

        // Restore es and bx.
        pop %bx
        pop %es

    // Get cached info.
    .cached_info:
        // %ds:di points to the pxe_cached_info structure.
        mov $pxe_cached_info, %di
        mov $GET_CACHED_INFO, %bx

        call pxe_api

        or (pxe_cached_info), %ax
        jnz .error_get_cached_info

        // Store the SIP & GIP over.
        mov %ax, %es
        mov $(pxe_cached_info + cached_info_sip), %di

        mov (pxe_cached_info + 6), %si
        add $20, %si        // Offset to the SIP and GIP.

        push %ds

        mov (pxe_cached_info + 8), %ax
        mov %ax, %ds

        movsl /* SIP */
        movsl /* GIP */

        pop %ds

    .return:
        popal
        ret

    .error_new_pxe:
        // Restore es and bx, and try PXENV+ structure.
        pop %bx
        pop %es
        jmp .pxenv

    .error_get_cached_info:
    .error_no_pxe:
        /* TODO: ERROR HANDLING. */
        hlt
        jmp .error_no_pxe

/*! Opens a file to be read from.
 *      \param esi points to the file name
 *      \return If file not found, eax non-zero. Else, aborts boot on error.
 */
.global file_open
file_open:
    xor %eax, %eax
    pushal
    push %ds

    mov %esi, %eax

    // Get the segment.
    shr $4, %eax
    mov %ax, %ds

    // And the offset.
    and $0x000F, %esi

    mov $(pxe_tftp_open + tftp_open_filename), %di
    mov $0x746f6f62 /* boot */, %eax
    stosl
    mov $0x2f, /* / */ %al
    stosb

    mov 127 - 5, %cx    // 5 bytes comprise of boot/.
    call strncpy

    xor %ax, %ax
    mov %ax, %ds    

    // Initialize pxe_tftp_open structure.
    movw %ax, (pxe_tftp_open)

    mov (pxe_cached_info + cached_info_sip), %eax
    mov %eax, (pxe_tftp_open + tftp_open_sip)
    mov (pxe_cached_info + cached_info_gip), %eax
    mov %eax, (pxe_tftp_open + tftp_open_gip)

    movl $0, (pxe_tftp_open + tftp_open_port)

    mov $pxe_tftp_open, %di
    mov $TFTP_GET_FSIZE, %bx
    call pxe_api

    orw (pxe_tftp_open), %ax
    jnz .open_fsize_error

    mov (pxe_tftp_open + tftp_open_port), %ecx
    mov %ecx, (file_size)

    movw $UDP_PORT, (pxe_tftp_open + tftp_open_port)
    movw $PACKET_SIZE, (pxe_tftp_open + tftp_open_packet_size)

    mov $pxe_tftp_open, %di
    mov $TFTP_OPEN, %bx
    call pxe_api

    orw (pxe_tftp_open), %ax
    jnz .open_fopen_error

    .open_ret:
        pop %ds
        popal
        ret

    .open_fsize_error:
    .open_fopen_error:
        cmpw $PXENV_STATUS_TFTP_FILE_NOT_FOUND, (pxe_tftp_open)
        jne .abort

        // Error out.
        pop %ds
        popal
        not %eax
        ret

    .abort:
        /* TODO: Error handling. */
        hlt        
        jmp .abort

/*! Reads the currently open file.
 *      \param edi where to read the file to
 *      \return Aborts boot on error.
 */
.global file_read
file_read:
    pushal

    movl (file_size), %ecx
    .read:
        mov %edi, %edx
        mov %edi, %ebx
        and $0x000F, %ebx
        shr $4, %edx
        // Segment in EDX, offset in EBX.

        mov %bx, (pxe_tftp_read + tftp_read_buffer_off)
        mov %dx, (pxe_tftp_read + tftp_read_buffer_seg)

        movl $0, (pxe_tftp_read + tftp_read_status)
        movw $0, (pxe_tftp_read + tftp_read_buffer_size)

        push %di

        mov $pxe_tftp_read, %di
        mov $TFTP_READ, %bx
        call pxe_api

        pop %di

    .read_loop:
        or (pxe_tftp_read), %ax
        jnz .read_error

        movzwl (pxe_tftp_read + tftp_read_buffer_size), %edx
        cmp $PACKET_SIZE, %edx
        jb .read_ret

        add %edx, %edi
        cmp %edx, %ecx
        jbe .read_ret

        sub %edx, %ecx
        jmp .read

    .read_ret:
        popal
        ret

    .read_error:
        /* TODO: Error handling. */
        hlt
        jmp .read_error

/*! Close the currently open file.
 *      \return Aborts boot on error.
 */
.global file_close  
file_close:
    pushal

    movw $0, (pxe_tftp_open)

    mov $pxe_tftp_open, %di
    mov $TFTP_CLOSE, %bx
    call pxe_api

    orw (pxe_tftp_open), %ax
    jnz .close_error

    .close_ret:
        popal
        ret

    .close_error:
        /* TODO: Error handling. */
        hlt
        jmp .close_error

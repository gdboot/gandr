/*
 * Disk module (for PXE).
 */

.code16
.data

// The PXE API pointer.
.align 4
pxe_api_addr:
    .offset:    .word 0
    .segment:   .word 0

#define PXE_NEW_PRESENT (1 << 0)

pxe_flags:
    .byte 0

#define UNDI_SHUTDOWN   0x05
#define UNLOAD_STACK    0x70
#define STOP_UNDI       0x15
#define UNDI_CLEANUP    0x02

#define GET_CACHED_INFO 0x71
#define TFTP_OPEN       0x20
#define TFTP_CLOSE      0x21
#define TFTP_READ       0x22
#define TFTP_GET_FSIZE  0x25

// The PXENV_GET_CACHED_INFO structure.
    .struct 0
cached_info_status:
    .struct cached_info_status + 2
cached_info_packet_type:
    .struct cached_info_packet_type + 2
cached_info_buffer_off:
    .struct cached_info_buffer_off + 2
cached_info_buffer_seg:
    .struct cached_info_buffer_seg + 2
cached_info_buffer_limit:
    .struct cached_info_buffer_limit + 2
cached_info_sip:
    .struct cached_info_sip + 4
cached_info_gip:

.data

// The PXENV_GET_CACHED_INFO structure.
pxe_cached_info:
    .status:                .word 0
    .packet_type:           .word 2
    .buffer_size:           .word 0
        .buffer_offset:     .word 0  // Zero implies PXE uses own buffer.
        .buffer_segment:    .word 0
    .buffer_limit:          .word 0
    
    // Not part of actual PXENV_GET_CACHED_INFO structure.
    .sip:                   .long 0
    .gip:                   .long 0

.text

/*
 * Call the PXE API (both before 2.1 and after).
 *     ds:di -> the address of the input buffer.
 *     bx -> the "opcode".
 */
pxe_api:
    // NOTE: 
    // The above registers are what the legacy API also used.
    // And we push it over here, so that we have no problems with the new API.
    push %ds
    push %di
    push %bx
    
    lcall *(pxe_api_addr)
    
    add $6, %sp
    ret

/*
 * Initialize PXE.
 *     es:bx -> the PXENV+ structure.
 *     ss:sp + 4 -> the !PXE structure (if valid).
 *
 * Trashes bp.
 */
.global storage_init
storage_init:
    // Save sp + 4.
    mov %sp, %bp
    add $4, %bp

    pushal

    // Check PXENV signature.
    cmpl $0x4E455850 /*PXEN*/, %es:(%bx)
    jne .error_no_pxe

    cmpw $0x2B56 /*V+*/, %es:4(%bx)
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
    jge .new_version

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

        cmpl $0x45585021 /*!PXE*/, %es:(%bx)
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
        // %es:di points to the pxe_cached_info structure.
        xor %ax, %ax
        mov %ax, %es
        mov $pxe_cached_info, %di

        mov $GET_CACHED_INFO, %bx

        call pxe_api

        or (pxe_cached_info), %ax
        jnz .error_get_cached_info

        // Store the SIP & GIP over.
        mov pxe_cached_info + cached_info_sip, %di

        mov (pxe_cached_info + cached_info_buffer_off), %si
        add $20, %si        // Offset to the SIP and GIP.

        push %ds

        mov (pxe_cached_info + cached_info_buffer_seg), %ax
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

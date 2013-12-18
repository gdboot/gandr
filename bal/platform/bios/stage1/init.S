/*
 * Stage 1 (disk dependent, firmware dependent) initialization code.
 */

.code16
.section base.start

/*
 * Entry point.
 *     dl -> the drive number.
 *     cs:ip -> the linear address 0x7C00.
 */
.global _start
_start:
    // Do a far jump to reset CS to 0x0000.
    ljmp $0x0000, $reset_cs

reset_cs:
    xor %ax, %ax

#ifndef BD_PXE
    // Set the stack just below where we start.
    // For PXE, the stack has already been set up.
    mov %ax, %ss
    mov $_start, %sp
#endif

    call output_init

    mov %ax, %ds

    call storage_init

    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    .hlt:
        hlt
        jmp .hlt

#ifdef BD_FLOPPY
// The boot signature.
.section boot_signature
    .word 0xAA55
#endif

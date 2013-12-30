/*
 * Stage 1 (disk dependent, firmware dependent) initialization code.
 */

.code16
.section base.start

#define BOOT_STACK  0x8000

/*
 * Entry point.
 *     dl -> the drive number.
 *     cs:ip -> the linear address 0x7C00.
 */
.global _start
_start:
    // Relocate us down to 0x0600.
#ifdef BD_PXE
    push %es
#endif

    xor %ax, %ax
    mov %ax, %es
    mov %ax, %ds

    mov $0x0600, %di
    mov $0x7C00, %si
    
#ifdef BD_PXE
    mov 0x2000 - 0x0600, %cx
#elseif defined(BD_ELTORITO)
    mov $2048, %cx
#else
    mov $512, %cx
#endif

    cld
    rep movsb

#ifdef BD_PXE
    pop %es
#endif

    // Do a far jump to reset CS to 0x0000.
    ljmp $0x0000, $reset_cs

reset_cs:
#ifndef BD_PXE
    // Set the stack just below where we start.
    // For PXE, there is already a stack set up.
    // And, storage_init requires %ss:%sp to be PXE-sane.
    mov %ax, %ss
    mov $BOOT_STACK, %sp
#endif

    call output_init
    call storage_init

#ifdef BD_PXE
    mov %ax, %ss
    mov $BOOT_STACK, %sp
#endif

    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    mov $0xDEADBEEF, %eax
    call output_hexadecimal

    .hlt:
        hlt
        jmp .hlt

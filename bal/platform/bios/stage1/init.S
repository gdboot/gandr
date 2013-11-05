/*
 * Stage 1 (disk independent) initialization code.
 */

.code16
.text

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

    // Set the stack.
    mov %ax, %ss
    mov $_start, %sp                   

    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    // Set mode (ah = 0x00) to 80*25 text mode (al = 0x03).
    mov $0x0003, %ax
    int $0x10

    .hlt:
        hlt
        jmp .hlt

// The boot signature.
.section boot_signature
    .word 0xAA55

#include <bal/bios_services.h>

.code16

.global api_table, stack_top, stack_base

// TEMP!
stack_top    = 0x8000
scratch_base = 0x8000

api_table:
    .ascii "BIOS"
    .long api_table_end - api_table
    .long stack_top
    .long scratch_base
    .word api_far_call, PM_CS16
    .word api_int_call, PM_CS16
api_table_end:

api_far_call:
    // Set registers as passed
    call pm16_to_rm
    popw %ds
    popw %es
    popw %fs
    popw %gs

    popal
    popf

    popl far_call_ptr
    lcall *far_call_ptr

    pushf
    pushal
    pushw %gs
    pushw %fs
    pushw %es
    pushw %ds
    call rm_to_pm16

    // Switch back to normal running
    mov $PM_DS32, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    movl stack_top - 4, %esp
    // Jump back into (32-bit/64-bit) code
    lretl
    .comm far_call_ptr, 4

api_int_call:
    call pm16_to_rm
    // Patch the int instruction
    mov %al, (1f+1)
    // Set registers as passed
    popw %ds
    popw %es
    popw %fs
    popw %gs

    popal
    popf

1:  int $0x00

    pushf
    pushal
    pushw %gs
    pushw %fs
    pushw %es
    pushw %ds
    call rm_to_pm16

    // Switch back to normal running
    mov $PM_DS32, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss
    movl stack_top - 4, %esp
    // Jump back into (32-bit/64-bit) code
    lretl


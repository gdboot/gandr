#include <bal/bios_services.h>

                .code16

                .section .data
                // Bits that need to be set to enter and exit protected mode
                // If we are loading a 64-bit BAL, these will be adjusted to set/clear
                // CR0.PG as well.
                .global pm_cr0_bits, pm_cr0_mask
                .align 4
pm_cr0_bits:    .long  0x00000001
pm_cr0_mask:    .long ~0x00000001

                .section .text
                .global rm_to_pm16, pm16_to_rm

// Switch to PM16
// Includes reloading all segment registers
// FS will be set to DS32
rm_to_pm16:     pushf
                cli
                push %eax

                mov %cr0, %eax
                or pm_cr0_bits, %eax
                mov %eax, %cr0

                mov $PM_DS16, %ax
                mov %ax, %ds
                mov %ax, %es
                mov %ax, %ss
                mov %ax, %gs
                mov $PM_DS32, %ax
                mov %ax, %fs

                ljmp $PM_CS16, $1f
1:              pop %eax
                popf
                ret

// Switch from PM16 to RM
// All segments will be reloaded with 16-bit limits
pm16_to_rm:     pushf
                cli
                push %eax
                movw $PM_DS16, %ax
                movw %ax, %ds
                movw %ax, %es
                movw %ax, %fs
                movw %ax, %gs
                movw %ax, %ss

                mov %cr0, %eax
                and pm_cr0_mask, %eax
                mov %eax, %cr0

                movw $RM_DS, %ax
                movw %ax, %ds
                movw %ax, %es
                movw %ax, %fs
                movw %ax, %gs
                movw %ax, %ss
                
                ljmp $RM_CS, $1f
1:              pop %eax
                popf
                ret



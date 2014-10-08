#include "asm_macros.h"
#include "aarch64.h"
#include "gd_elf.h"

                .global _start

func _start
                stp x0, x1, [sp, #-16]!
                mov x0, #0x04
                adr x1, hello
                hlt #0xF000
                ldp x0, x1, [sp], #16

                ldr w8, =DT_GANDR_ATFKICKSTART_VERSION
                cmp w0, w8
                bne .Lbad_magic

                cmp w1, #0
                bne .Lbad_version

                // Set stack pointer
                //ldr x0, =__estack
                //mov sp, x0

                mov x0, #0x04
                adr x1, hello
                hlt #0xF000
                mov x0, x2

                bl bal_main_atf

.Lbad_return:   mov x0, #0x04
                adr x1, .Lbad_return_s
                hlt #0xBA0
1:              wfi
                b 1b


.Lbad_magic:    mov x0, #0x04
                adr x1, .Lbad_magic_s
1:              wfi
                b 1b

.Lbad_version:  mov x0, #0x04
                adr x1, .Lbad_version_s
1:              wfi
                b 1b
endfunc _start
hello:          .asciz "Hello from gdbal\n"
.Lbad_return_s: .asciz "Bad return\n"
.Lbad_magic_s:  .asciz "Bad magic\n"
.Lbad_version_s:
                .asciz "Bad version\n"

                .section stack
                .space 65536
                __estack = .

                .section gddynamic
                .quad DT_GANDR_ATFKICKSTART_VERSION
                .quad 0

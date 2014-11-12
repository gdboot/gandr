/* Copyright © 2014, Owen Shepherd
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#include "asm_macros.h"
#include "aarch64.h"
                .section entrypoint
                .global _start
                .global shcall
                .global ks_go

.macro SetupEL ELX
                // Little endian FTW
                mrs x0, sctlr_\ELX
                bic x0, x0, #SCTLR_EE_BIT
                msr sctlr_\ELX, x0
                isb

                // enable icache, alignment checks, strict alignment checks
                mov    x1, #(SCTLR_I_BIT | SCTLR_A_BIT | SCTLR_SA_BIT)
                mrs    x0, sctlr_\ELX
                orr    x0, x0, x1
                msr    sctlr_\ELX, x0
                isb
.endm

func_special _start
                // If we are in EL2, set that up
                mrs x0, CurrentEL
                tbz x0, #3, Lin_el1

                // We are in EL2

                SetupEL EL2

                // Disable trapping from EL1 to EL2
                mrs    x0, cptr_el2
                bic    w0, w0, #TCPAC_BIT
                bic    w0, w0, #TTA_BIT
                bic    w0, w0, #TFP_BIT
                msr    cptr_el2, x0


Lin_el1:        SetupEL EL1

                // Zero BSS
                ldr x0, =__sbss
                ldr x1, =__ebss

1:              stp xzr, xzr, [x0], #16
                cmp x0, x1
                b.lt 1b

                // Set stack pointer
                ldr x0, =__estack
                mov sp, x0

                bl ks_main

1:              wfi
                b 1b
endfunc _start

/* Semihosting service call */
func shcall
                hlt #0xF000
                ret
endfunc shcall

func ks_go
                br x3
endfunc ks_go

                .section stack
                .balign 4096
                .space 4096

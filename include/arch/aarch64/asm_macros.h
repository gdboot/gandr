#ifndef ASM_MACROS_H
#define ASM_MACROS_H

				.macro func_special name
				.type \name, @function
\name:
				.endm

				.macro func name
				.section .text.\name
				func_special \name
				.endm

				.macro endfunc name
				.size \name, .-\name
				.endm

#endif

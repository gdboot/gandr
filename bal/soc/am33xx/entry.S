		.syntax unified
		.arm
		.section entry
		.globl _entry
_entry:	ldr sp, =(stack + 0x1000)
		b main

		.comm stack, 0x1000
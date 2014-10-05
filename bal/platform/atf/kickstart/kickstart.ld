OUTPUT_FORMAT(elf64-littleaarch64)
OUTPUT_ARCH(aarch64)
ENTRY(_start)
MEMORY {
    RAM (rwx): ORIGIN = 0x88000000, LENGTH = (0x100000000-0x88000000)
}
SECTIONS
{
    . = 0x88000000;
    .text : {
        __stext = .;
        *(entrypoint);
        *(.text*)
        *(.rodata*)
        __etext = .;
    } >RAM

    .data . : ALIGN(16) {
        __sdata = .;
        *(.data*)
        __edata = .;
    } >RAM

    .bss : ALIGN(16) {
        __sbss = .;
        *(.bss*)
        *(COMMON)
        __ebss = .;
    } >RAM

    stack . (NOLOAD) : {
        __sstack = .;
        *(stack)
        __estack = .;
    } >RAM

    . = ALIGN(4096);

    __startspace = .;
    __endspace   = ORIGIN(RAM) + LENGTH(RAM);

    pl011_regs = 0x001C090000;
}

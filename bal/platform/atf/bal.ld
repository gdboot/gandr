OUTPUT_FORMAT(elf64-littleaarch64)
OUTPUT_ARCH(aarch64)

ENTRY(_start)

SECTIONS
{
    stack . (NOLOAD) : ALIGN(4096) {
        *(stack)
    }

    .dynamic : {
        *(gddynamic)
        *(.dynamic*)
    }

    .text : {
        __stext = .;
        *(.text*)
        *(.rodata*)
        __etext = .;
    }

    .data . : ALIGN(16) {
        __sdata = .;
        *(.data*)
        __edata = .;
    }

    .bss : ALIGN(16) {
        __sbss = .;
        *(.bss*)
        *(COMMON)
        __ebss = .;
    }

    .rela.init      : { *(.rela.init) }
    .rela.text      : { *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*) }
    .rela.fini      : { *(.rela.fini) }
    .rela.rodata    : { *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*) }
    .rela.data.rel.ro   : { *(.rela.data.rel.ro .rela.data.rel.ro.* .rela.gnu.linkonce.d.rel.ro.*) }
    .rela.data      : { *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*) }
    .rela.tdata      : { *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*) }
    .rela.tbss      : { *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*) }
    .rela.ctors     : { *(.rela.ctors) }
    .rela.dtors     : { *(.rela.dtors) }
    .rela.got       : { *(.rela.got) }
    .rela.bss       : { *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*) }
    .rela.dyn       : { *(.rela.dyn*) }

    . = ALIGN(4096);
}

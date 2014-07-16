LD_FEATURE("SANE_EXPR")
ENTRY(_start)

MEMORY
{
    STAGE1      : ORIGIN = 0x0600, LENGTH = 0x2A00
}

SECTIONS
{
    .text :
    {
    _bootsector_start = .;
        *(base.start)
        *(base)
    _bootsector_end = .;
        *(.text*)
        *(.rodata*)
    } >STAGE1
 
    .data :
    {
        *(.data)
    } >STAGE1

    .bss :
    {
        bss_start = ABSOLUTE(.);
        *(.bss*)
        *(COMMON)
        bss_end = ABSOLUTE(.);
    } >STAGE1

    ASSERT(_bootsector_end - _bootsector_start < 2048, "Bootsector must be < 2048 bytes")
}
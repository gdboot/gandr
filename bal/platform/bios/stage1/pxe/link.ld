ENTRY(_start)

MEMORY
{
    STAGE1      : ORIGIN = 0x0600, LENGTH = 0x1A00
}

SECTIONS
{
    .text 0x0600 :
    {
        *(base.start)
        *(base)
        *(.text*)
        *(.rodata*)
    } >STAGE1
 
    .data :
    {
        *(.data)
        data_end = ABSOLUTE(.);
    } >STAGE1

    .bss :
    {
        bss_start = ABSOLUTE(.);
        *(.bss*)
        *(COMMON)
        bss_end = ABSOLUTE(.);
    } >STAGE1
}

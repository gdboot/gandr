ENTRY(_start)

MEMORY
{
    BOOT_SECTOR : ORIGIN = 0x0600, LENGTH = 0x800
    STAGE1      : ORIGIN = 0x0600, LENGTH = 0x2A00
}

SECTIONS
{
    .base 0x0600 :
    {
        *(base.start)
        *(base)
    } >BOOT_SECTOR

    .text :
    {
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
}

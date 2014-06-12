ENTRY(_start)
 
SECTIONS
{
    .text 0x0600 :
    {
        *(base.start)
        *(base)
        *(.text*)
        *(.rodata*)
    }
 
    .data :
    {
        *(.data)
    }

    .bss :
    {
        bss_start = .;
        *(.bss*)
        *(COMMON)
        bss_end = .;
    }
}

ENTRY(_start)
 
SECTIONS
{
    .text 0x0600 :
    {
        *(base.start)
        *(base)

        . = 2048 - 2;
        BYTE(0x55);
        BYTE(0xAA);

        *(.text*)
        *(.rodata*)
    }
 
    .data :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss*)
        *(COMMON)
    }
}

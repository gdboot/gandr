ENTRY(_start)
 
SECTIONS
{
    .text 0x0600 :
    {
        *(base.start)
        *(base)

        . = 0x01FE;
        BYTE(0x55);
        BYTE(0xAA);

        *(.text*)
        *(.rodata*)
    }
 
    .data :
    {
        *(.data)
        data_end = .;
    }

    .bss :
    {
        *(.bss*)
        *(COMMON)
    }
}

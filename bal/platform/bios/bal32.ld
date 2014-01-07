ENTRY(_start)
 
SECTIONS
{
    . = 0x10000;
    start = .;

    .text :
    {
        *(header)
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

    end = .;
}

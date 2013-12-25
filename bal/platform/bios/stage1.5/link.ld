ENTRY(_start)

SECTIONS
{
    .text 0x17000 :
    {
        *(header)
        *(.text)
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .bss :
    {
        bss_start = .;
        *(.bss)
        bss_end = .;
    }

    end = .;
}

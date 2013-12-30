ENTRY(_start)

SECTIONS
{
    /* This address is provisional */
    .text 0x8000 :
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
        _bss_start = .;
        *(.bss)
        _bss_end = .;
    }

    _end = .;
}

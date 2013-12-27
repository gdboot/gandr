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
        _bss_start = .;
        *(.bss)
        _bss_end = .;
    }

    _end = .;
}

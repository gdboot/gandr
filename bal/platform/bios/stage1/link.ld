OUTPUT_FORMAT("binary")
ENTRY(_start)

SECTIONS
{
    base 0x7C00 :
    {
        KEEP(*(base.start))
        *(base)
    }

    boot_signature 0x7DFC :
    {
        *(boot_signature)
    }

    .text :
    {
        *(.text)
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    signature 0x8FF8 :
    {
        *(signature)
    }

    end = .;
}

ENTRY(_start)

SECTIONS
{
    . = 0x10000;
    start = .;

    .text :
    {
        LONG(0x204c4142) /* "BAL " */
        LONG(_start)
        LONG(0x00000000)
        LONG(0x10000)
        LONG(0x00000000)
        LONG(bss_start)
        LONG(0x00000000)
        LONG(bss_end)
        LONG(0x00000000)
        LONG(0x00000000) /* CRC32 */

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

    end = .;
}

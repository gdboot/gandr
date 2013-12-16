OUTPUT_FORMAT("binary")
ENTRY(_start)
 
SECTIONS
{
    .text 0x7C00 :
    {
        *(base.start)
        *(base)
    }
 
    .data 0x7DFE :
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
 
    .data 0x8FF8 :
    {
        *(signature)
    }
 
    end = .;
}

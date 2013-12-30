ENTRY(_start)
 
SECTIONS
{
    .text 0x0600 :
    {
        *(base.start)
        *(base)
    }
 
    .data 0x07FE :
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
 
    .data 0x1FF8 :
    {
        *(signature)
    }
 
    end = .;
}

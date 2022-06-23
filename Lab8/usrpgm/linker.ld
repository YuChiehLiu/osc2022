SECTIONS
{
    . = 0x14000000;
    .text : {
        *(.text.start) 
        *(.text)
    }
    .data : 
    {
        *(.rodata)
        *(.data)
    }
    . = ALIGN(8);
    .bss : { *(.bss) }
    .= ALIGN(8);
}
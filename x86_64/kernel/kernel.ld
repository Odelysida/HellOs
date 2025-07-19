/* HellOS Kernel Linker Script */

ENTRY(_start)

SECTIONS
{
    . = 0x8000;
    
    .text : {
        *(.text)
    }
    
    .data : {
        *(.data)
    }
    
    .bss : {
        *(.bss)
    }
    
    /DISCARD/ : {
        *(.comment)
        *(.note)
    }
} 
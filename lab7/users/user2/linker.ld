ENTRY(_start)

SECTIONS {
    .text : { *(.text.start) *(.text) } /* text.start is a custom section. To specify the first funtion to execute */
    . = ALIGN(0x1000);
    .rodata : { *(.rodata) }
    . = ALIGN(0x1000);
    .data : { *(.data) }                /* Initialized data section (.data) */
    . = ALIGN(0x1000);
    .bss : { *(.bss) }                  /* Uninitialized data section (.bss) */
    . = ALIGN(0x1000);
    _end = .; 
}
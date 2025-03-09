ENTRY(_start)

SECTIONS {
    . = 0x80000;          /* Start address where the program will be loaded */

    .text : { *(.text.boot) *(.text) } /* text.boot is custom section. To specify bootloader code */
    .rodata : { *(.rodata) }
    .data : { *(.data) }            /* Initialized data section (.data) */

    /* Uninitialized data section (.bss) */
    .bss (NOLOAD) : {
        . = ALIGN(16);       /* Align the section to 16-byte boundary */
        __bss_start = .;     /* Mark the start of .bss */
        *(.bss)              
        *(COMMON)            /* Include common symbols (uninitialized globals) */
        __bss_end = .;       /* Mark the end of .bss */
    }

    .heap (NOLOAD) : {
        . = ALIGN(16);
        _heap_start = .;  /* Start of the heap */
        . = . + 4M;       /* Allocate 4MB for the heap */
        _heap_end = .;    /* End of the heap */
    }

    .stack (NOLOAD) : {
        . = ALIGN(16);
        _stack_start = .;       /* Mark the start of the stack */
        . = . + 4M;             /* Allocate 4MB for the stack */
        _stack_end = .;         /* Mark the end of the stack */
    }

    _end = .;               /* Mark the end of the program's memory layout */
}
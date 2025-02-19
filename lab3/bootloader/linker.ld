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

    .stack (NOLOAD) : {
        . = ALIGN(16);
        _stack_start = .;       /* Mark the start of the stack */
        . = . + 0x4000;       /* Allocate 4KB for the stack */
        _stack_end = .;         /* Mark the end of the stack */
    }

    _end = .;               /* Mark the end of the program's memory layout */
    
    /*_bootloader_load_addr = 0x100000;   /* Bootloader start address */
    /*_kernel_load_addr = _start;       /* Kernel load address */
}
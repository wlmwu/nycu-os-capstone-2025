#include "bootloader.h"

extern unsigned long _start;
extern unsigned long _end;

extern fdt_header_t *g_fdt_header_start;

void bootloader_run() {
    void (*loadfptr)() = bootloader_load;
    bootloader_relocate();
    void (*new_loadfptr)() = (void(*)(void))(BOOTLOADER_LOAD_ADDR + ((unsigned long)loadfptr - KERNEL_LOAD_ADDR));     // Adjust function pointer to bootloader's relocated address
    
    new_loadfptr();
}

void bootloader_load() {
    uart_puts("Waiting for kernel image...\n");

    // Receive kernel size (4 bytes)
    unsigned int kernel_size = 0;
    kernel_size = uart_getu();

    char ssize[33];     /* 4 bytes + '\0' */
    itos(ssize, kernel_size, 10);
    uart_puts("Kernel size received: ");
    uart_puts(ssize);
    uart_puts(" bytes\n");

    // Receive kernel data
    char *kernel = (char*)(KERNEL_LOAD_ADDR);
    for (unsigned int i = 0; i < kernel_size; i++) {
        kernel[i] = uart_getc();
    }

    uart_puts("Kernel loaded successfully...\n");

    // Jump to the kernel
    ((void (*)()) kernel)((void*)g_fdt_header_start);
}

void bootloader_relocate() {
    unsigned long int start = (unsigned long int)(&_start);
    if (start == BOOTLOADER_LOAD_ADDR) {
        return;
    }
    uart_puts("Relocation starts...\n");

    unsigned long int end = (unsigned long int)(&_end);
    unsigned long int size = end - KERNEL_LOAD_ADDR;

    char *bootloader_addr = (char*)(BOOTLOADER_LOAD_ADDR);
    char *start_addr = (char*)(KERNEL_LOAD_ADDR);

    for (unsigned long int i = 0; i < size; ++i) {
        bootloader_addr[i] = start_addr[i];
    }

    uart_puts("Relocation finished.\n");
}
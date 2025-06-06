#include "bootloader.h"
#include "mini_uart.h"
#include <stdint.h>

extern void *g_fdt_header;

void bootloader_load() {
    uart_puts("Waiting for kernel image...\n");

    // Receive kernel size (4 bytes)
    uint32_t kernel_size = 0;
    uint32_t checksum = 0;
    kernel_size = uart_getu();
    checksum = uart_getu();

    uart_printf("Kernel size received: %u bytes\n", kernel_size);

    uart_puts("Receiving kernel image...\n");

    // Receive kernel data
    char *kernel = (void*)(KERNEL_LOAD_ADDR);
    for (uint32_t i = 0; i < kernel_size; ++i) {        
        kernel[i] = uart_getc();
    }
    
    uint32_t sum = 0;
    for (uint32_t i = 0; i < kernel_size; ++i) {
        sum += (uint32_t)kernel[i];
    }

    if (sum != checksum) {
        uart_printf("Error: Checksum mismatch! Calculated: %x, Expected: %x\n", sum, checksum);
        return;
    }

    uart_puts("Kernel loaded successfully!\n");

    // Jump to the kernel
    ((void (*)()) kernel)((void*)g_fdt_header);
}
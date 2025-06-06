#include "bootloader.h"
#include "mini_uart.h"
#include "fdt.h"
#include "utils.h"

extern fdt_header_t *g_fdt_header_start;

void bootloader_load() {
    uart_puts("Waiting for kernel image...\n");

    // Receive kernel size (4 bytes)
    uint32_t kernel_size = 0;
    uint32_t checksum = 0;
    kernel_size = uart_getu();
    checksum = uart_getu();

    char ssize[33];     /* 4 bytes + '\0' */
    itos(ssize, kernel_size, 10);
    uart_puts("Kernel size received: ");
    uart_puts(ssize);
    uart_puts(" bytes\n");

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
        uart_puts("Error: Checksum mismatch!\n");
        return;
    }

    uart_puts("Kernel loaded successfully!\n");

    // Jump to the kernel
    ((void (*)()) kernel)((void*)g_fdt_header_start);
}
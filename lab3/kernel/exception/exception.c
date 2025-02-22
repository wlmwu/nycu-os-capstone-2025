#include "exception.h"

void el1t_64_sync_handler(void *regs) {}
void el1t_64_irq_handler(void *regs) {}
void el1t_64_fiq_handler(void *regs) {}
void el1t_64_error_handler(void *regs) {}

void el1h_64_sync_handler(void *regs) {}
void el1h_64_irq_handler(void *regs) {}
void el1h_64_fiq_handler(void *regs) {}
void el1h_64_error_handler(void *regs) {}

void el0t_64_sync_handler(void *regs) {
    uint64_t spsr_el1, elr_el1, esr_el1;

    // Read system registers using inline assembly
    asm volatile (
        "mrs %0, spsr_el1 \n\t"
        "mrs %1, elr_el1  \n\t"
        "mrs %2, esr_el1  \n\t"
        : "=r" (spsr_el1), "=r" (elr_el1), "=r" (esr_el1) // Output operands
        :  // No input operands
    );

    // Print register values
    uart_puts("SPSR_EL1: 0x");
    uart_puts((char*)uint2hexstr(NULL, (unsigned int)spsr_el1));
    uart_puts("\n");
    uart_puts("ELR_EL1: 0x");
    uart_puts((char*)uint2hexstr(NULL, (unsigned int)elr_el1));
    uart_puts("\n");
    uart_puts("ESR_EL1: 0x");
    uart_puts((char*)uint2hexstr(NULL, (unsigned int)esr_el1));
    uart_puts("\n\n");
}
void el0t_64_irq_handler(void *regs) {}
void el0t_64_fiq_handler(void *regs) {}
void el0t_64_error_handler(void *regs) {}

void el0t_32_sync_handler(void *regs) {}
void el0t_32_irq_handler(void *regs) {}
void el0t_32_fiq_handler(void *regs) {}
void el0t_32_error_handler(void *regs) {}
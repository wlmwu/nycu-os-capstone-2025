#include "exception.h"
#include "mini_uart.h"
#include "irq.h"
#include <stdint.h>
#include <stddef.h>

void el1t_64_sync_handler(void *regs) {}
void el1t_64_irq_handler(void *regs) {}
void el1t_64_fiq_handler(void *regs) {}
void el1t_64_error_handler(void *regs) {}

void el1h_64_sync_handler(void *regs) {}
void el1h_64_irq_handler(void *regs) {
    irq_handle();
}
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

    irq_enable();   // DAIF is reset to 0x3c0 after each svc.

    uart_printf("SPSR_EL1:\t%p,\t", spsr_el1);
    uart_printf("ELR_EL1:\t%p,\t", elr_el1);
    uart_printf("ESR_EL1:\t%p,\t", esr_el1);
    uart_puts("\n");
}
void el0t_64_irq_handler(void *regs) {
    irq_handle();
}
void el0t_64_fiq_handler(void *regs) {}
void el0t_64_error_handler(void *regs) {}

void el0t_32_sync_handler(void *regs) {}
void el0t_32_irq_handler(void *regs) {}
void el0t_32_fiq_handler(void *regs) {}
void el0t_32_error_handler(void *regs) {}
#include "exception.h"
#include "mini_uart.h"
#include <stdint.h>
#include <stddef.h>

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
    uart_printf("SPSR_EL1:\t%p\n", spsr_el1);
    uart_printf("ELR_EL1:\t%p\n", elr_el1);
    uart_printf("ESR_EL1:\t%p\n", esr_el1);
    uart_puts("\n");
}
void el0t_64_irq_handler(void *regs) {
    const int kTimeoutSeconds = 2;

    unsigned long cntpct, cntfrq;
    asm volatile(
        "mrs %[pct], cntpct_el0\n\t"                // Get the timer's current count (total number of ticks since system boot)
        "mrs %[frq], cntfrq_el0"                    // Read the timer frequency
        : [pct] "=r"(cntpct), [frq] "=r"(cntfrq) 
    );

    unsigned int current_time = cntpct / cntfrq;    // The time (in second) passed since system boot
    uart_puts("Time passed after booting: ");
    uart_putu(current_time);
    uart_puts(" secs\n");

    unsigned long timeout_value = cntfrq * kTimeoutSeconds;  
    
    asm volatile(
        "msr    cntp_tval_el0, %[tval]     \n"      // Set the next expiration time
        "ldr    x1, =0x40000040            \n"      // 0x40000040: Core 0 Timers interrupt control
        "mov    w0, #2                     \n"
        "str    w0, [x1]                   \n"
        :
        : [tval] "r" (timeout_value)
    );
    
}
void el0t_64_fiq_handler(void *regs) {}
void el0t_64_error_handler(void *regs) {}

void el0t_32_sync_handler(void *regs) {}
void el0t_32_irq_handler(void *regs) {}
void el0t_32_fiq_handler(void *regs) {}
void el0t_32_error_handler(void *regs) {}
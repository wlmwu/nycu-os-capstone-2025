#include "exception.h"
#include "mini_uart.h"
#include "irq.h"
#include "sched.h"
#include "syscall.h"
#include "signal.h"
#include <stdint.h>
#include <stddef.h>

void el1t_64_sync_handler(void *regs) {
    uart_dbg_printf("el1t_64_sync_handler ");
}
void el1t_64_irq_handler(void *regs) {}
void el1t_64_fiq_handler(void *regs) {}
void el1t_64_error_handler(void *regs) {}

void el1h_64_sync_handler(void *regs) {
    uart_dbg_printf("el1h_64_sync_handler ");
}
void el1h_64_irq_handler(trapframe_t *tf) {
    irq_handle();
    signal_check(tf);
}
void el1h_64_fiq_handler(void *regs) {}
void el1h_64_error_handler(void *regs) {}

void el0t_64_sync_handler(trapframe_t *tf) {
    irq_enable();   // DAIF is reset to 0x3c0 after each svc.
    uint64_t esr;
    __asm__ volatile("mrs %0, esr_el1": "=r" (esr));
    if (EXCEPT_CLASS(esr) == EC_SVC) {
        syscall_handle(tf);
    } else {
        uart_dbg_printf("\033[0;31mError: Unhandled exceptioin class %x, ESR_EL1: %x, Thread: %p\033[0m\n", EXCEPT_CLASS(esr), esr, sched_get_current());
        sched_get_current()->sigpending |= SIGKILL;
    }
    signal_check(tf);
}
void el0t_64_irq_handler(trapframe_t *tf) {
    irq_handle();
    signal_check(tf);
}
void el0t_64_fiq_handler(void *regs) {}
void el0t_64_error_handler(void *regs) {}

void el0t_32_sync_handler(void *regs) {}
void el0t_32_irq_handler(void *regs) {}
void el0t_32_fiq_handler(void *regs) {}
void el0t_32_error_handler(void *regs) {}
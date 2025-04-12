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
    syscall_handle(tf);
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
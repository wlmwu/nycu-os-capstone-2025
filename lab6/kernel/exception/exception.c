#include "exception.h"
#include "mini_uart.h"
#include "irq.h"
#include "sched.h"
#include "syscall.h"
#include "signal.h"
#include "vm.h"
#include <stdint.h>
#include <stddef.h>

void el1t_64_sync_handler(void *regs) {
    uart_dbg_printf("el1t_64_sync_handler ");
}
void el1t_64_irq_handler(void *regs) {}
void el1t_64_fiq_handler(void *regs) {}
void el1t_64_error_handler(void *regs) {}

void el1h_64_sync_handler(void *regs) {
    esr_el1_t esr;
    __asm__ volatile("mrs %0, esr_el1": "=r" (esr));
    uart_dbg_printf("\033[0;31mEL1h: Unhandled exceptioin class %x, ESR_EL1: %x, Thread: %p\033[0m\t", esr.ec, sched_get_current());
}
void el1h_64_irq_handler(trapframe_t *tf) {
    irq_handle();
    signal_check(tf);
}
void el1h_64_fiq_handler(void *regs) {}
void el1h_64_error_handler(void *regs) {}

void el0t_64_sync_handler(trapframe_t *tf) {
    irq_enable();   // DAIF is reset to 0x3c0 after each svc.
    
    esr_el1_t esr;
    __asm__ volatile("mrs %0, esr_el1": "=r" (esr));
    
    int retval = -1;
    if (esr.ec == EC_SVC_64) {       // System call
        retval = syscall_handle(tf);
    } else if (esr.ec == EC_DATA_ABORT_LOWER_EL || esr.ec == EC_INSTR_ABORT_LOWER_EL) {   // Page fault
        uint64_t far;
        __asm__ volatile("mrs %0, far_el1": "=r" (far));
        retval = vm_fault_handle(far, esr);
    } else {
        uart_dbg_printf("\033[0;31mError: Unhandled exceptioin class %x, ESR_EL1: %x, Thread: %p\033[0m\n", esr.ec, esr.value, sched_get_current());
    }
    if (retval < 0) sched_get_current()->sigpending |= SIGKILL;
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
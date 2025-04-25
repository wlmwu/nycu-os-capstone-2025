#include "mini_uart.h"
#include "shell.h"
#include "cpio.h"
#include "fdt.h"
#include "irq.h"
#include "timer.h"
#include "memory.h"
#include "sched.h"
#include "kthread.h"
#include "utils.h"
#include "syscall.h"

int main(void* arg) {   /* The value of arg is `x0` which is 0x8200000 in QEMU, so `*arg` is the pointer points to 0x8200000 */
    uart_init();
    timer_init();

    irq_init();
    
    fdt_init(arg);
    cpio_init();
    
    memory_init();

    sched_init();

    shell_init();
    shell_run();        // Always runs in EL1

    return 0;
}
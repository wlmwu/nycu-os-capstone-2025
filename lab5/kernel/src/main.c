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

static void delay(unsigned int clock) {
    while (clock--) {
        asm volatile("nop");
    }
}

static void foo(void *args) {
    int n = *(int*)args;
    for(int i = 0; i < n; ++i) {
        uart_printf("Thread ID %d: %d\n", (unsigned long)get_current(), i);
        delay(1000000);
        schedule();
    }
}

int main(void* arg) {   /* The value of arg is `x0` which is 0x8200000 in QEMU, so `*arg` is the pointer points to 0x8200000 */
    uart_init();
    timer_init();

    irq_init();
    
    fdt_init(arg);
    cpio_init();
    
    memory_init();

    sched_init();

    shell_init();
    // shell_run();
    for (int i = 0; i < 5; ++i) {
        int *n = kmalloc(sizeof(int));
        *n = i + 1;
        kthread_t *t = kthread_run(foo, n);
    }
    kthread_run(shell_run, 0);
    schedule();

    return 0;
}
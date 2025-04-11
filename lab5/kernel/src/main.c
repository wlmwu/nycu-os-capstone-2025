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

static inline void call_sys_yield() {
    asm volatile(
        "mov    x8, %[x8]   \n"
        "svc    0           \n" 
        :: [x8] "r" (SYS_YIELD)
    );
}

static void foo(void *args) {
    int n = *(int*)args;
    for(int i = 0; i < n; ++i) {
        uart_printf("Thread ID %d: %d\n", (unsigned long)sched_get_current(), i);
        call_sys_yield();
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
    // for (int i = 0; i < 5; ++i) {
    //     int *n = kmalloc(sizeof(int));
    //     *n = i + 1;
    //     kthread_run(foo, n);
    // }
    shell_run();        // Always runs in EL1

    return 0;
}
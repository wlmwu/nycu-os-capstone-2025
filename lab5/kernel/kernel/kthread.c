#include "kthread.h"
#include "slab.h"
#include "sched.h"
#include "list.h"
#include "utils.h"
#include "syscall.h"
#include "mini_uart.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

static inline void kthread_syscall_exit() {
    asm volatile(
        "mov    x8, %[x8]   \n"
        "svc    0           \n" 
        :: [x8] "r" (SYS_EXIT)
    );
}

static void kthread_fn_wrapper() {
    sched_task_t *thrd = sched_get_current();
    if (!thrd) return;
    thrd->fn(thrd->args);
    // kthread_exit();
    kthread_syscall_exit();     // All threads run in EL0, so use syscall to exit 
}

sched_task_t* kthread_create(sched_fn_t fn, void *args) {
    sched_task_t *thrd = kmalloc(sizeof(sched_task_t));
    memset(thrd, 0, sizeof(sched_task_t));

    thrd->state = kThRunnable;
    thrd->stack_bottom = kmalloc(SCHED_STACK_SIZE);
    thrd->fn = fn;
    thrd->args = args;
    INIT_LIST_HEAD(&thrd->list);
    
    uint64_t *stack_top = thrd->stack_bottom + SCHED_STACK_SIZE;      // top: high addr, bottom: low addr
    thrd->context.pc = (unsigned long)kthread_fn_wrapper;
    thrd->context.sp = (unsigned long)stack_top;
    thrd->context.fp = (unsigned long)thrd->context.sp;
    // uart_dbg_printf("Kthread %x creates stack %p\n", thrd, thrd->context.sp);

    return thrd;
}

void kthread_exit() {
    sched_task_t *curr = sched_get_current();
    curr->state = kThDead;
    asm volatile(
        "msr    spsr_el1, xzr      \n"  // EL0t mode, IRQ enable
        "msr    elr_el1, %[pc]     \n"  // Set LR back to schedule 
        "eret                      \n"  // Return to EL0
        :
        : [pc] "r" ((uintptr_t)schedule)
    );
}

sched_task_t* kthread_run(sched_fn_t fn, void *args) {
    sched_task_t *thrd = kthread_create(fn, args);
    sched_enqueue_task(thrd);
    return thrd;
}
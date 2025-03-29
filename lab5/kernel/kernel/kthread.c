#include "kthread.h"
#include "slab.h"
#include "sched.h"
#include "list.h"
#include "utils.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

static void kthread_fn_wrapper() {
    kthread_t *thrd = get_current();
    if (!thrd) return;
    thrd->fn(thrd->args);
    kthread_exit();
}

kthread_t* kthread_create(kthread_fn_t fn, void *args) {
    kthread_t *thrd = kmalloc(sizeof(kthread_t));
    memset(thrd, 0, sizeof(kthread_t));

    thrd->state = kThRunnable;
    thrd->stack_bottom = kmalloc(KTHREAD_STACK_SIZE);
    thrd->fn = fn;
    thrd->args = args;
    INIT_LIST_HEAD(&thrd->list);
    
    uint64_t *stack_top = thrd->stack_bottom + KTHREAD_STACK_SIZE;      // top: high addr, bottom: low addr
    thrd->context.pc = (unsigned long)kthread_fn_wrapper;
    thrd->context.sp = (unsigned long)stack_top;
    thrd->context.fp = (unsigned long)thrd->context.sp;

    return thrd;
}

void kthread_exit() {
    kthread_t *curr = get_current();
    curr->state = kThDead;
    schedule();
}

kthread_t* kthread_run(kthread_fn_t fn, void *args) {
    kthread_t *thrd = kthread_create(fn, args);
    sched_enqueue_task(thrd);
    return thrd;
}
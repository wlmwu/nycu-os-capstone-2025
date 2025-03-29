#include "sched.h"
#include "kthread.h"
#include "list.h"
#include "slab.h"
#include "mini_uart.h"
#include "utils.h"

static LIST_HEAD(sched_queue);      // contain all threads

extern void cpu_switch_to(void *prev_ctx, void *next_ctx);      // defined in entry.S
static void context_switch(kthread_t *prev, kthread_t *next) {  
    set_current(next);
    cpu_switch_to(&prev->context, &next->context);
}

void schedule() {
    if (list_empty(&sched_queue)) {
        uart_printf("Error: run queue is empty\n");
        return;
    }

    kthread_t *curr = get_current();
    kthread_t *next = list_entry(sched_queue.next, kthread_t, list);
    list_del(&next->list);
    list_add_tail(&next->list, &sched_queue);

    context_switch(curr, next);
}

void sched_enqueue_task(kthread_t *thread) {
    list_add_tail(&thread->list, &sched_queue);
}

static void idle() {
    while (1) {
        kthread_t *thrd, *tmp;
        list_for_each_entry_safe(thrd, tmp, &sched_queue, list) {
            if (thrd->state == kThDead) {
                list_del(&thrd->list);
                kfree(thrd->stack_bottom);
                kfree(thrd);
            }
        }
        schedule();
    }
} 

void sched_init() {
    kthread_t *thrd = kthread_run(idle, NULL);
}
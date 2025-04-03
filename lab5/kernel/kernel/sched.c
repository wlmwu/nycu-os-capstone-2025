#include "sched.h"
#include "kthread.h"
#include "list.h"
#include "slab.h"
#include "mini_uart.h"
#include "utils.h"

static LIST_HEAD(sched_queue);      // Contain all threads

extern void cpu_switch_to(void *prev_ctx, void *next_ctx);      // Defined in entry.S
static void context_switch(sched_task_t *prev, sched_task_t *next) {  
    set_current(next);
    cpu_switch_to(&prev->context, &next->context);
}

void schedule() {
    if (list_empty(&sched_queue)) {
        uart_printf("Error: run queue is empty\n");
        return;
    }

    sched_task_t *curr = get_current();
    sched_task_t *next = list_entry(sched_queue.next, sched_task_t, list);
    list_del(&next->list);
    list_add_tail(&next->list, &sched_queue);

    context_switch(curr, next);
}

void sched_enqueue_task(sched_task_t *thread) {
    list_add_tail(&thread->list, &sched_queue);
}

static void idle() {
    while (1) {
        sched_task_t *thrd, *tmp;
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
    sched_task_t *thrd = kthread_run(idle, NULL);
}
#include "sched.h"
#include "kthread.h"
#include "list.h"
#include "slab.h"
#include "mini_uart.h"
#include "utils.h"
#include "slab.h"
#include "syscall.h"
#include "shell.h"
#include "timer.h"
#include "irq.h"
#include <stdbool.h>
#include "mmu.h"
#include "proc.h"

static LIST_HEAD(sched_queue);      // Contain all threads
static sched_task_t init;           // Placeholder task to avoid writing to 0x0 on the first context switch

extern void cpu_switch_to(void *prev_ctx, void *next_ctx);      // Defined in entry.S
static void context_switch(sched_task_t *prev, sched_task_t *next) {  
    sched_set_current(next);
    mmu_switch_to(next->pgd);
    cpu_switch_to(&prev->context, &next->context);
}

void schedule() {
    if (list_empty(&sched_queue)) {
        return;
    }

    sched_task_t *curr = sched_get_current();
    sched_task_t *next = list_entry(sched_queue.next, sched_task_t, list);
    list_del(&next->list);
    while (next->state == kThDead) {
        list_add_tail(&next->list, &sched_queue);
        next = list_entry(sched_queue.next, sched_task_t, list);
        list_del(&next->list);
    }

    if (list_empty(&sched_queue)) {         // All threads have exited (except `idle()`), return to shell
        shell_run();
    } else {
        list_add_tail(&next->list, &sched_queue);
        context_switch(curr, next);
    }
}

void sched_enqueue_task(sched_task_t *thread) {
    list_add_tail(&thread->list, &sched_queue);
}

extern unsigned long _start;
extern unsigned long _end;
static inline bool is_user_fn(void *fn) {
    return !((uintptr_t)fn >= (uintptr_t)(&_start) && (uintptr_t)fn <= (uintptr_t)(&_end));
}  

void idle() {
    while (1) {
        sched_task_t *thrd, *tmp;
        list_for_each_entry_safe(thrd, tmp, &sched_queue, list) {
            if (thrd->state == kThDead) {
                list_del(&thrd->list);
                // if (is_user_fn(thrd->fn)) kfree(thrd->fn);   // Parent might exit before children. However, they must use the same memory for the program.
                kfree(thrd->ustack);
                kfree(thrd->kstack);
                kfree(thrd);
            }
        }
    }
} 

static void periodic_schedule() {
    uint64_t freq = timer_get_current_freq();
    timer_add_event(periodic_schedule, NULL, 0, freq >> 5);
    schedule();
}

void sched_init() {
    sched_set_current(&init);
    timer_add_event(periodic_schedule, NULL, 0, 1);
}

void sched_start() {
    sched_set_current(&init);
    sched_task_t *thrd = kthread_create(idle, NULL);    // Run `idle` in EL1 
    thrd->context.pc = (uintptr_t)idle;
    thrd->pgd = PGTABLE_START_ADDR;
    sched_enqueue_task(thrd);
}

sched_task_t* sched_get_task(int taskid) {
    sched_task_t *thrd, *tmp;
    list_for_each_entry_safe(thrd, tmp, &sched_queue, list) {
        if ((uintptr_t)thrd == taskid) {    // PID hasn't implemented, use address instead
            return thrd;
        }
    }
    return NULL;
}

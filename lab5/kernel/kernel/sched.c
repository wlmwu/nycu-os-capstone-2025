#include "sched.h"
#include "kthread.h"
#include "list.h"
#include "slab.h"
#include "mini_uart.h"
#include "utils.h"
#include "slab.h"
#include <stdbool.h>

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

    // uart_dbg_printf("Switch: %p (sp: %x) -> %p (sp: %x)\n", curr, curr->context.sp, next, next->context.sp);

    context_switch(curr, next);
}

void sched_enqueue_task(sched_task_t *thread) {
    list_add_tail(&thread->list, &sched_queue);
}

extern unsigned long _start;
extern unsigned long _end;
static inline bool is_user_fn(void *fn) {
    return !((uintptr_t)fn >= (uintptr_t)(&_start) && (uintptr_t)fn <= (uintptr_t)(&_end));
}  

static void idle() {
    while (1) {
        sched_task_t *thrd, *tmp;
        list_for_each_entry_safe(thrd, tmp, &sched_queue, list) {
            if (thrd->state == kThDead) {
                list_del(&thrd->list);
                if (is_user_fn(thrd->fn)) kfree(thrd->fn);
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

void sched_start() {
    void *kstack = kmalloc(SCHED_STACK_SIZE);       // Create a kernel stack for save/load operations during EL transitions
    uint64_t ksp = (uint64_t)kstack + SCHED_STACK_SIZE;
    // uart_dbg_printf("Schedule creates stack %p\n", ksp);
    asm volatile(
        "msr    sp_el0, %[sp]   \n"     // Set user stack pointer
        "msr    elr_el1, %[pc]  \n"     // Set return address to user program
        "msr    spsr_el1, xzr   \n"     // Enable interrupt in EL0
        "eret                   \n"
        :
        : [sp] "r" (ksp), [pc] "r" ((uintptr_t)schedule)
    );
}
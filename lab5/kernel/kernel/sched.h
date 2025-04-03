#ifndef SCHED_H_
#define SCHED_H_

#include "list.h"

#define SCHED_STACK_SIZE 4096
struct sched_context {
    unsigned long x19;
	unsigned long x20;
	unsigned long x21;
	unsigned long x22;
	unsigned long x23;
	unsigned long x24;
	unsigned long x25;
	unsigned long x26;
	unsigned long x27;
	unsigned long x28;
	unsigned long fp;
	unsigned long pc;
	unsigned long sp;
};

typedef void (*sched_fn_t)(void *args);

typedef struct sched_task {
    struct sched_context context;
    enum { kThRunnable, kThDead } state;
    void *stack_bottom;
    sched_fn_t fn;
    void *args;
	struct list_head list;
} sched_task_t;

void schedule();
void sched_enqueue_task(sched_task_t *thread);
void sched_init();

static inline sched_task_t* get_current() {
    void *curr_thrd;
    asm volatile("mrs %0, tpidr_el1" : "=r"(curr_thrd));
    return curr_thrd;
}

static inline void set_current(sched_task_t* thread) {
    asm volatile("msr tpidr_el1, %0" :: "r"((unsigned long)thread));
}

#endif // SCHED_H_

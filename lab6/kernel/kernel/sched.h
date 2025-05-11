#ifndef SCHED_H_
#define SCHED_H_

#include "list.h"
#include "signal.h"
#include "exception.h"
#include <stdint.h>
#include <stddef.h>

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
    struct sched_context context;			// Saved CPU context for context switch
    enum { kThRunnable, kThDead } state;	// Current state
    void *kstack;                           // Kernel stack bottom
    sched_fn_t fn;							// Entry point address of the user program
    void *args;								// Argument passed to the user program's entry point (fn)
    size_t size;							// Size of the user program
	sighandler_t sighandlers[NSIG];			// Array of signal handlers registered by the user
    uint64_t sigpending;					// Bitmask of pending signals for the task
    trapframe_t *sigcontext;				// Saved context before signal handler execution
    uint64_t pgd;                           // PGD (physical address) of the process
    struct list_head vm_area_queue;         // List head to link all `vm_area_t`s
	struct list_head list;					// List node to link all tasks
} sched_task_t;

void schedule();
void sched_enqueue_task(sched_task_t *thread);

void sched_start();
void sched_init();

sched_task_t* sched_get_task(uint32_t taskid);

static inline sched_task_t* sched_get_current() {
    void *curr_thrd;
    asm volatile("mrs %0, tpidr_el0" : "=r"(curr_thrd));
    return curr_thrd;
}

static inline void sched_set_current(sched_task_t* thread) {
    asm volatile("msr tpidr_el0, %0" :: "r"((unsigned long)thread));
}

#endif // SCHED_H_

#ifndef KTHREAD_H_
#define KTHREAD_H_

#include <stddef.h>
#include <stdint.h>
#include "list.h"

#define KTHREAD_STACK_SIZE 4096

struct kthread_context {
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

typedef void (*kthread_fn_t)(void *args);

typedef struct kthread {
    struct kthread_context context;
    enum { kThRunnable, kThDead } state;
    void *stack_bottom;
    kthread_fn_t fn;
    void *args;
	struct list_head list;
} kthread_t;


kthread_t *kthread_create(kthread_fn_t fn, void *args);
void kthread_exit();

kthread_t *kthread_run(kthread_fn_t fn, void *args);

static inline kthread_t* get_current() {
    void *curr_thrd;
    asm volatile("mrs %0, tpidr_el1" : "=r"(curr_thrd));
    return curr_thrd;
}

static inline void set_current(kthread_t* thread) {
    asm volatile("msr tpidr_el1, %0" :: "r"((unsigned long)thread));
}

#endif // KTHREAD_H_

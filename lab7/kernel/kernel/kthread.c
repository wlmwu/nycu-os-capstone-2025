#include "kthread.h"
#include "slab.h"
#include "sched.h"
#include "list.h"
#include "utils.h"
#include "syscall.h"
#include "mini_uart.h"
#include "proc.h"
#include "mmu.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

static inline void call_sys_exit() {
    asm volatile(
        "mov    x8, %[x8]   \n"
        "svc    0           \n" 
        :: [x8] "r" (SYS_EXIT)
    );
}

static void kthread_fn_wrapper() {
    sched_task_t *thrd = sched_get_current();
    if (!thrd) return;
    asm volatile(
        "msr    sp_el0, %[usp]  \n"     // Set user stack pointer
        "msr    elr_el1, %[pc]  \n"     // Set return address to user program
        "msr    spsr_el1, xzr   \n"     // Enable interrupt in EL0
        "mov    sp, %[ksp]      \n"     // When SVC, `sp` restores immediately to what it was before ERET (i.e., `ksp` here). So this `ksp` is used for the save/load operations during EL transition
        "mov    lr, %[lr]       \n"     // Afte ERET, program jumps to `lr`, so user program can exit without explicitly calling `exit()`
        "mov    x0, %[args]     \n"     
        "eret                   \n"
        :: [usp] "r" (PROC_USTACK_BASE + PROC_STACK_SIZE), [pc] "r" (PROC_ENTRY_POINT), 
           [ksp] "r" (thrd->kstack + PROC_STACK_SIZE), [lr] "r" ((uintptr_t)call_sys_exit),
           [args] "r" (thrd->args), [pgd] "r" (PA_TO_VA(thrd->pgd))
    );
}

sched_task_t* kthread_create(sched_fn_t fn, void *args) {
    sched_task_t *thrd = kmalloc(sizeof(sched_task_t));
    memset(thrd, 0, sizeof(sched_task_t));

    static pid_t pid = 0;
    thrd->pid = pid++;
    thrd->state = kThRunnable;
    thrd->kstack = kmalloc(PROC_STACK_SIZE);
    thrd->fn = fn;
    thrd->args = args;
    INIT_LIST_HEAD(&thrd->list);
    
    uint64_t *stack_top = thrd->kstack + PROC_STACK_SIZE;      // `kthread_fn_wrapper` needs a stack and `kstack` is unused at that time.
    thrd->context.pc = (unsigned long)kthread_fn_wrapper;
    thrd->context.sp = (unsigned long)stack_top;
    thrd->context.fp = (unsigned long)thrd->context.sp;

    memset(thrd->sighandlers, 0, sizeof(thrd->sighandlers));
    thrd->sigpending = 0ULL;
    thrd->sigcontext = NULL;

    thrd->pgd = VA_TO_PA(kmalloc(PAGE_SIZE));
    memset((void*)PA_TO_VA(thrd->pgd), 0, PAGE_SIZE);

    INIT_LIST_HEAD(&thrd->vm_area_queue);

    thrd->cwd = NULL;
    memset(thrd->fdtable, 0, sizeof(thrd->fdtable));
    
    return thrd;
}

void kthread_exit() {
    sched_task_t *curr = sched_get_current();
    curr->state = kThDead;
    schedule();
}

sched_task_t* kthread_run(sched_fn_t fn, void *args) {
    sched_task_t *thrd = kthread_create(fn, args);
    sched_enqueue_task(thrd);
    return thrd;
}
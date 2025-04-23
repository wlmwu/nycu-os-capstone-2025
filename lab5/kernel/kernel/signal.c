#include "signal.h"
#include "sched.h"
#include "syscall.h"
#include "slab.h"

void signal_check(trapframe_t *tf) {
    sched_task_t *curr = sched_get_current();
    for (int sig = 0; sig < NSIG && curr->sigpending; ++sig) {
        if (curr->sigpending & (1 << sig)) {
            curr->sigpending &= ~(1 << sig);
            signal_handle(sig, tf);
            break;
        }
    }
}

static inline void call_sys_sigreturn() {
    asm volatile(
        "mov    x8, %[x8]   \n"
        "svc    0           \n" 
        :: [x8] "r" (SYS_SIGRETURN)
    );
}

static void signal_handle_wrapper(sighandler_t handler, int signo) {
    handler(signo);
    call_sys_sigreturn();
}

void signal_handle(int signo, trapframe_t *tf) {
    sched_task_t *curr = sched_get_current();
    sighandler_t handler = curr->sighandlers[signo];
    if (handler == SIG_DFL) {
        curr->state = kThDead;
        schedule();
    } else if (handler != SIG_IGN) {
        curr->sigcontext = kmalloc(sizeof(trapframe_t));
        *curr->sigcontext = *tf;
        tf->elr = (uintptr_t)signal_handle_wrapper;
        tf->x[0] = (uintptr_t)handler;
        tf->x[1] = signo;
    }
}
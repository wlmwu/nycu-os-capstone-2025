#include "syscall.h"
#include "sched.h"
#include "kthread.h"
#include "exception.h"
#include "mini_uart.h"
#include "utils.h"
#include "cpio.h"
#include "slab.h"
#include "mailbox.h"
#include "irq.h"

static int sys_getpid(trapframe_t *tf) {
    sched_task_t *curr = sched_get_current();
    uint32_t pid = (uintptr_t)curr;
    return pid;
}

static int sys_read(trapframe_t *tf) {
    char *ptr = (char*)(tf->x[0]);
    size_t sz = tf->x[1];
    while (sz--) {
        char c = uart_getc();
        *ptr++ = c;
    }
    return tf->x[1];
}

static int sys_write(trapframe_t *tf) {
    char *str = (char*)(tf->x[0]);
    size_t sz = tf->x[1];
    while ((*str) && (sz--)) {
        uart_printf("\033[38;5;153m%c\033[0m", *str++);
    }
    
    return tf->x[1];
}

static int sys_exec(trapframe_t *tf) {
    char *filename = (char*)(tf->x[0]);
    cpio_newc_header_t *hptr = cpio_get_file_by_name(filename);
    if (!hptr) {
        uart_printf("%s: No such file or directory\n", filename);
        return -1;
    }

    char *filedata;
    uint32_t filesize = 0;
    cpio_get_file(hptr, NULL, &filesize, &filedata);
    
    void *prog = kmalloc(filesize);
    memcpy(prog, filedata, filesize);

    sched_task_t *curr = sched_get_current();
    curr->size = filesize;
    kfree(curr->fn);                            // Assume `curr` is a user process

    curr->fn = prog;
    tf->sp = (uintptr_t)curr->ustack + SCHED_STACK_SIZE;
    tf->elr = (uintptr_t)curr->fn;
    return tf->x[0];
}

static int sys_fork(trapframe_t *tf) {
    irq_disable();
    sched_task_t *parent = sched_get_current();
    sched_task_t *child = kthread_run(parent->fn, parent->args);
    child->size = parent->size;

    memcpy(child->sighandlers, parent->sighandlers, sizeof(parent->sighandlers));
    child->sigpending = parent->sigpending;

    int32_t kstack_offset = (int32_t)child->kstack - (int32_t)parent->kstack;
    int32_t ustack_offset = (int32_t)child->ustack - (int32_t)parent->ustack;
    memcpy(child->kstack, parent->kstack, SCHED_STACK_SIZE);
    memcpy(child->ustack, parent->ustack, SCHED_STACK_SIZE);
    uintptr_t sp, fp;
    asm volatile(
        "mov %0, sp     \n" 
        "mov %1, fp     \n"
        : "=r" (sp), "=r" (fp)
        :
    );

    child->context = parent->context;
    child->context.fp = (uintptr_t)((int32_t)fp + kstack_offset);  
    child->context.sp = (uintptr_t)((int32_t)sp + kstack_offset);
    *(uintptr_t*)child->context.fp += kstack_offset;        // Adjust `fp` to child's stack copy

childret:
    sched_task_t *curr = sched_get_current();
    if (curr == parent) {
        child->context.pc = (uintptr_t)(&&childret);    // https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html
        irq_enable();
        return (uintptr_t)child;
    } else {
        tf = (trapframe_t*)((char*)tf + kstack_offset);
        tf->sp = (int32_t)tf->sp + ustack_offset;
        tf->x[0] = 0;
        return 0;       // Return to the parent's trapframe, since `syscall_handle` was called with the parent's trapframe as an argument
    }
}

static int sys_exit(trapframe_t *tf) {
    kthread_exit();
    return 0;
}

static int sys_mboxcall(trapframe_t *tf) {
    unsigned char channel = tf->x[0];
    unsigned int *mbox = (unsigned int*)(tf->x[1]);
    return mbox_call(mbox, channel);
}

static int sys_kill(trapframe_t *tf) {
    int pid = tf->x[0];
    sched_task_t *thrd = sched_get_task(pid);
    if (!thrd) return -1;
    thrd->state = kThDead;
    return 0;
}

static int sys_yield(trapframe_t *tf) {
    schedule();
    return 0;
}

static int sys_signal(trapframe_t *tf) {
    int sig = tf->x[0];
    sighandler_t handler = (sighandler_t)tf->x[1];
    sched_task_t *curr = sched_get_current();
    if (sig < 0 || sig >= NSIG) return -1;
    curr->sighandlers[sig] = handler;
    return 0;
}

static int sys_sigkill(trapframe_t *tf) {
    int pid = tf->x[0];
    int sig = tf->x[1];
    sched_task_t *thrd = sched_get_task(pid);
    if (!thrd || sig < 0 || sig >= NSIG) return -1;
    thrd->sigpending |= (1 << sig);
    return 0;
}

static int sys_sigreturn(trapframe_t *tf) {
    sched_task_t *curr = sched_get_current();
    if (curr->sigcontext) {
        *tf = *curr->sigcontext;
        kfree(curr->sigcontext);
        curr->sigcontext = NULL;
    }
    return 0;
}

static int (*syscalls[])(trapframe_t *tf) = {
    [SYS_GETPID]    sys_getpid,
    [SYS_READ]      sys_read,
    [SYS_WRITE]     sys_write,
    [SYS_EXEC]      sys_exec,
    [SYS_FORK]      sys_fork,
    [SYS_EXIT]      sys_exit,
    [SYS_MBOXCALL]  sys_mboxcall,
    [SYS_KILL]      sys_kill,
    [SYS_YIELD]     sys_yield,
    [SYS_SIGNAL]    sys_signal,
    [SYS_SIGKILL]   sys_sigkill,
    [SYS_SIGRETURN] sys_sigreturn,
};

void syscall_handle(trapframe_t *tf) {
    sched_task_t *current = sched_get_current();
    int sysnum = tf->x[8];

    if ((sysnum >= 0) && 
        (sysnum < sizeof(syscalls) / sizeof(syscalls[0])) &&
        syscalls[sysnum]) {
        tf->x[0] = syscalls[sysnum](tf);
    } else {
        uart_dbg_printf("\033[0;31mError: Unknown syscall %x\033[0m\n");
        tf->x[0] = -1;
        sys_exit(tf);
    }
}

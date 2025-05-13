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
#include "proc.h"
#include "mmu.h"
#include "vm.h"

static int sys_getpid(trapframe_t *tf) {
    sched_task_t *curr = sched_get_current();
    uint32_t pid = (uintptr_t)curr;
    return pid;
}

static int sys_read(trapframe_t *tf) {
    char *ptr = (char*)(tf->x[0]);
    size_t sz = tf->x[1];
    while (sz--) {
        char c = uart_async_getc();
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
    void *prog = NULL;
    size_t progsize = 0;
    proc_load_prog(filename, &prog, &progsize);

    sched_task_t *curr = sched_get_current();
    
    curr->fn = prog;
    curr->size = progsize;
    
    vm_release(curr);
    proc_setup_vma(curr, prog, progsize);

    memset(curr->sighandlers, 0, sizeof(curr->sighandlers));

    tf->sp = PROC_USTACK_BASE + PROC_STACK_SIZE;
    tf->elr = PROC_ENTRY_POINT;
    
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
    memcpy(child->kstack, parent->kstack, PROC_STACK_SIZE);
    vm_copy(child, parent);
    vm_map_pages(child, PROC_FRAMEBUF_PTR, PROC_FRAMEBUF_PTR, PROC_FRAMEBUF_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC);      // Require designated physical address mapping

    uintptr_t sp, fp;
    asm volatile(
        "mov %0, sp     \n" 
        "mov %1, fp     \n"
        : "=r" (sp), "=r" (fp)
        :
    );

    child->context = parent->context;
    child->context.fp = PA_TO_VA((int32_t)fp + kstack_offset);  
    child->context.sp = PA_TO_VA((int32_t)sp + kstack_offset);
    *(uintptr_t*)child->context.fp += kstack_offset;        // Adjust `fp` to child's stack copy

childret:
    sched_task_t *curr = sched_get_current();
    if (curr == parent) {
        child->context.pc = (uintptr_t)(&&childret);    // https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html
        irq_enable();
        return (uintptr_t)child;
    } else {
        tf = (trapframe_t*)((char*)tf + kstack_offset);
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

    uint64_t par;
    asm volatile (
        "at     s1e0r, %[va]      \n"       // Stage 1 EL0 Read translation
        "isb                      \n"       // Ensure par_el1 is updated
        "mrs    %[par], par_el1   \n"
        : [par] "=r" (par)
        : [va] "r" (mbox)
        : "memory"
    );

    uint64_t addr = (par & PAGE_MASK) | ((uint64_t)mbox & (PAGE_SIZE - 1));   // Extract physical address
    int retval = mbox_call((void*)PA_TO_VA(addr), channel);

    return retval;
}

static int sys_kill(trapframe_t *tf) {
    uint32_t pid = tf->x[0];
    sched_task_t *thrd = sched_get_task(pid);
    if (!thrd) {
        uart_printf("kill: (%u) - No such process\n", pid);
        return -1;
    }
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

static int sys_mmap(trapframe_t *tf) {
    sched_task_t *curr = sched_get_current();
    uint64_t addr = tf->x[0];
    size_t len = tf->x[1];
    int prot = tf->x[2];
    int flags = tf->x[3];
    int fd = tf->x[4];
    int file_offset = tf->x[5];

    return (int)vm_mmap(curr, addr, len, prot, flags, fd, file_offset);
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
    [SYS_MMAP]      sys_mmap,
};

int syscall_handle(trapframe_t *tf) {
    sched_task_t *current = sched_get_current();
    int sysnum = tf->x[8];

    if ((sysnum >= 0) && 
        (sysnum < sizeof(syscalls) / sizeof(syscalls[0])) &&
        syscalls[sysnum]) {
        tf->x[0] = syscalls[sysnum](tf);
    } else {
        uart_dbg_printf("\033[0;31mError: Unknown syscall %x\033[0m\n");
        tf->x[0] = -1;
        return -1;
    }
    return 0;
}

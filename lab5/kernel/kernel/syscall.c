#include "syscall.h"
#include "sched.h"
#include "kthread.h"
#include "exception.h"
#include "mini_uart.h"
#include "utils.h"

int sys_getpid(unsigned long args[]) {
    uart_dbg_printf("\033[1;95mSyscall GetPID\033[0m\n");
    sched_task_t *curr = get_current();
    uint32_t pid = (uintptr_t)curr;
    return pid;
}
int sys_read(unsigned long args[]) {
    uart_dbg_printf("\033[1;95mSyscall Read\033[0m\n");
    char *ptr = (char*)(args[0]);
    size_t sz = args[1];
    while (sz--) {
        char c = uart_getc();
        uart_printf("%c", c);
        *ptr++ = c;
    }
    return args[1];
}
int sys_write(unsigned long args[]) {
    uart_dbg_printf("\033[1;95mSyscall Write\033[0m\n");
    char *str = (char*)(args[0]);
    while (*str) {
        uart_printf("\033[1;33m%c\033[0m", *str++);
    }
    
    return args[1];
}
int sys_exec(unsigned long args[]) {
    uart_dbg_printf("Syscall Unimplemented\n");
    return 0;
}
int sys_fork(unsigned long args[]) {
    uart_dbg_printf("Syscall Unimplemented\n");
    return 0;
}
int sys_exit(unsigned long args[]) {
    uart_dbg_printf("\033[1;95mSyscall Exit\033[0m\n");
    kthread_exit();
    return 0;
}
int sys_mboxcall(unsigned long args[]) {
    uart_dbg_printf("Syscall Unimplemented\n");
    return 0;
}
int sys_kill(unsigned long args[]) {
    uart_dbg_printf("Syscall Unimplemented\n");
    return 0;
}

static int (*syscalls[])(unsigned long[]) = {
    [SYS_GETPID]    sys_getpid,
    [SYS_READ]      sys_read,
    [SYS_WRITE]     sys_write,
    [SYS_EXEC]      sys_exec,
    [SYS_FORK]      sys_fork,
    [SYS_EXIT]      sys_exit,
    [SYS_MBOXCALL]  sys_mboxcall,
    [SYS_KILL]      sys_kill,
};

void syscall_handle(trapframe_t *tf) {
    sched_task_t *current = get_current();
    int sysnum = tf->x[8];
    uart_dbg_printf("Sysnum %u, arg0: %x\n", tf->x[8], tf->x[0]);

    if ((sysnum >= 0) && 
        (sysnum < sizeof(syscalls) / sizeof(syscalls[0])) &&
        syscalls[sysnum]) {
        tf->x[0] = syscalls[sysnum](tf->x);
    } else {
        uart_dbg_printf("\033[1;31m[ERROR] Unknown syscall %x\033[0m\n");
        tf->x[0] = -1;
        sys_exit(tf->x);
    }
}

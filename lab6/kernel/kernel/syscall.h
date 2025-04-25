#ifndef SYSCALL_H_
#define SYSCALL_H_

#include "exception.h"

#define SYS_GETPID      0
#define SYS_READ        1
#define SYS_WRITE       2
#define SYS_EXEC        3
#define SYS_FORK        4
#define SYS_EXIT        5
#define SYS_MBOXCALL    6
#define SYS_KILL        7
#define SYS_SIGNAL      8
#define SYS_SIGKILL     9
#define SYS_SIGRETURN   10
#define SYS_YIELD       11       // To call schedule() in EL0

void syscall_handle(trapframe_t *tf);


#endif // SYSCALL_H_

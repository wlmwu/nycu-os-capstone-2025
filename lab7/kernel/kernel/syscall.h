#ifndef SYSCALL_H_
#define SYSCALL_H_

#include "exception.h"

#define SYS_GETPID      0
#define SYS_GETS        1
#define SYS_PUTS        2
#define SYS_EXEC        3
#define SYS_FORK        4
#define SYS_EXIT        5
#define SYS_MBOXCALL    6
#define SYS_KILL        7
#define SYS_SIGNAL      8
#define SYS_SIGKILL     9
#define SYS_MMAP        10
#define SYS_OPEN        11
#define SYS_CLOSE       12
#define SYS_WRITE       13
#define SYS_READ        14
#define SYS_MKDIR       15
#define SYS_MOUNT       16
#define SYS_CHDIR       17
#define SYS_SIGRETURN   18
#define SYS_YIELD       19       // To call schedule() in EL0

int syscall_handle(trapframe_t *tf);


#endif // SYSCALL_H_

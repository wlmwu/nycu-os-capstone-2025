#ifndef SYSCALL_H_
#define SYSCALL_H_

#include <stdint.h>
#include <stddef.h>

#define SYS_GETPID      0
#define SYS_READ        1
#define SYS_WRITE       2
#define SYS_EXEC        3
#define SYS_FORK        4
#define SYS_EXIT        5
#define SYS_MBOXCALL    6
#define SYS_KILL        7

static unsigned long syscall(unsigned long sysnum, unsigned long x[]) {
    unsigned long retval;
    asm volatile(
        "mov    x8, %[x8]    \n" 
        "mov    x0, %[x0]    \n"
        "mov    x1, %[x1]    \n" 
        "mov    x2, %[x2]    \n" 
        "mov    x3, %[x3]    \n" 
        "mov    x4, %[x4]    \n" 
        "mov    x5, %[x5]    \n" 
        "mov    x6, %[x6]    \n" 
        "mov    x7, %[x7]    \n" 
        "svc    0            \n"
        "mov    %[retv], x0  \n" 
        : [retv] "=r" (retval)
        : [x8] "r" (sysnum), [x0] "r" (x[0]), [x1] "r" (x[1]), [x2] "r" (x[2]), [x3] "r" (x[3]), 
          [x4] "r" (x[4]), [x5] "r" (x[5]), [x6] "r" (x[6]), [x7] "r" (x[7])
    );

    return retval;
}


static inline int getpid() {
    unsigned long x[8];   // x0 ~ x7
    unsigned long retval = syscall(SYS_GETPID, x);
    return retval;
}

static inline size_t write(const char buf[], size_t size) {
    unsigned long x[8];   // x0 ~ x7
    x[0] = (uintptr_t)buf;
    x[1] = size;
    unsigned long retval = syscall(SYS_WRITE, x);

    return retval;
}

static inline size_t read(const char buf[], size_t size) {
    unsigned long x[8];   // x0 ~ x7
    x[0] = (uintptr_t)buf;
    x[1] = size;
    unsigned long retval = syscall(SYS_READ, x);

    return retval;
}

static inline void exit() {
    unsigned long x[8];   // x0 ~ x7
    unsigned long retval = syscall(SYS_EXIT, x);
}

#endif // SYSCALL_H_
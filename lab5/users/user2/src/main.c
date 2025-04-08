#include <stdint.h>
#include <stddef.h>

#include "syscall.h"
#include "io.h"

#define READ_LEN 64

__attribute__((section(".text.start"))) int main() {
    asm volatile("mov x27, 0x72");  // Used to track the progress of the program when using gdb 

    int pid = getpid();
    asm volatile("mov x27, 0x721");

    printf("PID %d\n", pid);
    asm volatile("mov x27, 0x722");

    puts("hello world !!!!\n");
    asm volatile("mov x27, 0x723");
    
    char buf[READ_LEN];
    fgets(buf, READ_LEN);
    puts(buf);
    asm volatile("mov x27, 0x724");

    exec("syscall.img", NULL);

    // exit();
    return 0;
}
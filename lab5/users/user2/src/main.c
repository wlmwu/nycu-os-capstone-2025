#include <stdint.h>
#include <stddef.h>

#include "syscall.h"
#include "io.h"

#define READ_LEN 64

static void fork_test(){
    printf("\nFork Test, pid %d\n", getpid());
    int cnt = 1;
    int ret = 0;
    if ((ret = fork()) == 0) { // child
        long long cur_sp;
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        ++cnt;

        if ((ret = fork()) != 0){
            asm volatile("mov %0, sp" : "=r"(cur_sp));
            printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
        }
        else{
            while (cnt < 5) {
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n", getpid(), cnt, &cnt, cur_sp);
                ++cnt;
            }
        }
        exit();
    }
    else {
        printf("parent here, pid %d, child %d\n", getpid(), ret);
    }
}

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

    if ((pid = fork()) == 0) {  // child
        puts("Child\n");
        exec("syscall.img", NULL);
    } else {
        puts("Parent\n");
        fork_test();
    }

    printf("PID %d exit\n", getpid());

    // exit();
    return 0;
}
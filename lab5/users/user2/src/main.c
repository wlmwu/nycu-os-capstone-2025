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

static int get_board_revision() {
    unsigned int  __attribute__((aligned(16))) buf[7];
    buf[0] = 7 * 4;     
    buf[1] = 0;                                              
    buf[2] = 0x00010002;
    buf[3] = 4;                  
    buf[4] = 0;             
    buf[5] = 0;                                                        
    buf[7] = 0;      

    mbox_call(8, buf);
    return buf[5];
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
    
    printf("PID %d Starts Fork Test\n", getpid());
    
    fork_test();
    
    printf("PID %d Starts Kill Test\n", getpid());

    int child1, child2;
    if ((child1 = fork()) == 0) {  // child
        printf("Child 1 %d\n", getpid());
        if ((child2 = fork()) == 0) {  // child
            printf("Child 2 %d\n", getpid());
        } else {
            printf("Child 1 %d forked Child 2 %d\n", getpid(), child2);
            kill(child2);
            printf("Child 1 %d killed Child 2 %d\n", getpid(), child2);
        }
        exec("syscall.img", NULL);
    } else {
        printf("Parent %d\n", getpid());
        printf("Board Revision:\t%x\n", get_board_revision());
    }

    // exit();
    return 0;
}
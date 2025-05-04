#include "proc.h"
#include "sched.h"
#include "kthread.h"
#include "mmu.h"
#include "cpio.h"
#include "mini_uart.h"
#include "utils.h"
#include "slab.h"
#include "vm.h"
#include "irq.h"
#include <stdint.h>

// Attributes used in video player
#define FRAMEBUF_PTR    0x3c100000      // FrameBufferInfo.pointer
#define FRAMEBUF_SIZE   0x300000        // FrameBufferInfo.size

int proc_load_prog(char *filename, void **prog, size_t *progsize) {
    if (!filename || !prog || !progsize) {
        uart_printf("Error: Arguments should not be NULL\n");
        return -1;
    }

    cpio_newc_header_t *hptr = cpio_get_file_by_name(filename);
    if (!hptr) {
        uart_printf("%s: No such file or directory\n", filename);
        return -1;
    }

    char *filedata;
    cpio_get_file(hptr, NULL, (unsigned int*)progsize, &filedata);
    
    *prog = kmalloc((*progsize));
    memcpy(*prog, filedata, (*progsize));

    return 0;
}

sched_task_t* proc_create(void *prog, void *args, size_t progsize) {
    sched_task_t *thrd = kthread_create(prog, args);
    thrd->size = progsize;
    
    vma_add(thrd, PROC_ENTRY_POINT, PROC_ENTRY_POINT + progsize,        PROT_READ | PROT_WRITE | PROT_EXEC, VA_TO_PA(prog));
    vma_add(thrd, PROC_USTACK_BASE, PROC_USTACK_BASE + PROC_STACK_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, 0);
    vma_add(thrd, FRAMEBUF_PTR,     FRAMEBUF_PTR + FRAMEBUF_SIZE,       PROT_READ | PROT_WRITE | PROT_EXEC, FRAMEBUF_PTR);

    sched_enqueue_task(thrd);
    return thrd;
}
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

    vm_map_pages(thrd, PROC_ENTRY_POINT, VA_TO_PA(prog), progsize, PD_AP_RW_EL0);
    vm_map_pages(thrd, PROC_USTACK_BASE, VA_TO_PA(thrd->ustack), PROC_STACK_SIZE, PD_AP_RW_EL0);
    vm_map_pages(thrd, FRAMEBUF_PTR, FRAMEBUF_PTR, FRAMEBUF_SIZE, PD_AP_RW_EL0);
    
    sched_enqueue_task(thrd);
    return thrd;
}
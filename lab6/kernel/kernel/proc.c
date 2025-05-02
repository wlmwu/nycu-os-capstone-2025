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

    vm_map_pages(thrd, PROC_ENTRY_POINT, VA_TO_PA(prog), progsize, 0);
    vm_map_pages(thrd, PROC_USTACK_BASE, VA_TO_PA(thrd->ustack), PROC_STACK_SIZE, 0);
    
    sched_enqueue_task(thrd);
    return thrd;
}
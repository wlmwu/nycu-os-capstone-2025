#include "proc.h"
#include "sched.h"
#include "kthread.h"
#include "mmu.h"
#include "cpio.h"
#include "mini_uart.h"
#include "utils.h"
#include "slab.h"
#include "vm.h"
#include "vfs.h"
#include "fs.h"
#include "errno.h"
#include <stdint.h>

int proc_load_prog(char *filename, void **prog, size_t *progsize) {
    if (!filename || !prog || !progsize) {
        uart_printf("Error: Arguments should not be NULL\n");
        return -EINVAL;
    }
    fs_file_t *file;
    int retval = vfs_open(fs_get_root(), filename, O_RDONLY, &file);
    if (retval < 0) {
        if (retval == -ENOENT) uart_printf("%s: No such file or directory\n", filename);
        return retval;
    }
    
    fs_vattr_t attr;
    retval = vfs_getattr(file->vnode, &attr);
    if (retval < 0) return retval;

    *prog = (void*)file;
    *progsize = attr.size;

    return 0;
}

sched_task_t* proc_create(void *prog, void *args, size_t progsize) {
    sched_task_t *thrd = kthread_create(prog, args);
    thrd->size = progsize;
    proc_setup_vma(thrd, prog, progsize);
    proc_setup_fs(thrd);
    sched_enqueue_task(thrd);
    return thrd;
}

void proc_setup_vma(sched_task_t *thrd, void *prog, size_t progsize) {
    vma_add(thrd, PROC_ENTRY_POINT, PROC_ENTRY_POINT + progsize, PROT_READ | PROT_WRITE | PROT_EXEC, (uint64_t)prog);
    vma_add(thrd, PROC_USTACK_BASE, PROC_USTACK_BASE + PROC_STACK_SIZE, PROT_READ | PROT_WRITE | PROT_EXEC, 0);
}

void proc_setup_fs(sched_task_t *thrd) {
    fs_vnode_t *root = fs_get_root();
    fs_file_t *stdin, *stdout, *stderr;
    vfs_open(root, "/dev/uart", O_RDONLY, &stdin);
    vfs_open(root, "/dev/uart", O_WRONLY, &stdout);
    vfs_open(root, "/dev/uart", O_WRONLY, &stderr);
    
    thrd->cwd = root;
    thrd->fdtable[0] = stdin;
    thrd->fdtable[1] = stdout;
    thrd->fdtable[2] = stderr;
}

void proc_release(sched_task_t *thrd) {
    vm_release(thrd);
    for (int fd = 0; fd < PROC_NUM_FDTABLE; ++fd) {
        if (thrd->fdtable[fd]) {
            vfs_close(thrd->fdtable[fd]);
        }
    }
    kfree(thrd->kstack);
    kfree((void*)PA_TO_VA(thrd->pgd));
    kfree(thrd);
}
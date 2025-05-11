#ifndef PROC_H_
#define PROC_H_

#include "mmu.h"

#define PROC_STACK_SIZE     (PAGE_SIZE * 4)    
#define PROC_USTACK_BASE    0xffffffffb000    
#define PROC_ENTRY_POINT    0x0 

// Attributes used in video player
#define PROC_FRAMEBUF_PTR    0x3c100000      // FrameBufferInfo.pointer
#define PROC_FRAMEBUF_SIZE   0x300000        // FrameBufferInfo.size

#include "sched.h"
#include <stdint.h>

int proc_load_prog(char *filename, void **prog, size_t *progsize);

void proc_setup_vma(sched_task_t *thrd, void *prog, size_t progsize);

sched_task_t* proc_create(void *prog, void *args, size_t progsize);
void proc_release(sched_task_t *thrd);

#endif // PROC_H_

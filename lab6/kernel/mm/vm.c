#include "vm.h"
#include "utils.h"
#include "buddy.h"
#include "mini_uart.h"
#include "proc.h"
#include "slab.h"
#include <stdbool.h>

vm_area_t* vma_add(sched_task_t *thrd, uint64_t start, uint64_t end, uint64_t prot, uint64_t file) {
    vm_area_t *vma = kmalloc(sizeof(vm_area_t));
    vma->start = start;
    vma->end = end;
    vma->prot = prot;
    vma->file = file;
    list_add_tail(&vma->list, &thrd->vm_area_queue);

    return vma;
}

static bool vma_is_free(sched_task_t *task, uint64_t start, uint64_t end) {
    vm_area_t *vma, *tmp;
    list_for_each_entry_safe(vma, tmp, &task->vm_area_queue, list) {
        if (!(start >= vma->end || end <= vma->start)) {
            return false;
        }
    }
    return true;
}

static uint64_t vma_find_free(sched_task_t *task, uint64_t hint, size_t len) {
    if (hint && vma_is_free(task, hint, hint + len)) {     // Hint is specified and hint is available
        return hint;
    }

    for (uint64_t base = ALIGN(PROC_ENTRY_POINT + task->size, PAGE_SIZE); base + len < PROC_USTACK_BASE; base += len) {     // Search an address between program and ustack 
        if (vma_is_free(task, base, base + len)) {
            return base;
        }
    }

    return 0;
}

static uint64_t prot_to_flag(uint64_t prot) {
    uint64_t pte_flags = 0;
    if ((prot & PROT_READ) && (prot & PROT_WRITE)) pte_flags |= PD_AP_RW_EL0;
    else if (prot & PROT_READ) pte_flags |= PD_AP_RO_EL0;
    if (!(prot & PROT_EXEC)) pte_flags |= PD_UXN;

    return pte_flags;
}

static void map_page(sched_task_t *thrd, uint64_t va, uint64_t page, uint64_t flag) {
    if (!thrd->pgd) {
        thrd->pgd = (uint64_t)page_alloc(0);
        memset((void*)PA_TO_VA(thrd->pgd), 0, PAGE_SIZE);
    }
    uint64_t *table = (uint64_t*)PA_TO_VA(thrd->pgd);
    for (int level = 0; level < 3; ++level) {     
        uint64_t idx = PAGE_IDX(level, va);         // PGD(0) -> PUD(1) -> PMD(2)
        if (!table[idx]) {
            uint64_t *new_table = (uint64_t*)page_alloc(0);
            memset((void*)PA_TO_VA(new_table), 0, PAGE_SIZE);
            table[idx] = ((uint64_t)new_table) | PD_TYPE_TABLE;
        }
        table = (uint64_t*)PA_TO_VA((uint64_t)(table[idx] & (~((uint64_t)(PAGE_SIZE) - 1))));
    }

    uint64_t idx = PAGE_IDX(3, va);                 // PTE(3)
    table[idx] = page | PD_AF | PD_MAIR_NORMAL_NOCACHE | PD_TYPE_PAGE | flag;
}

void vm_map_pages(sched_task_t *thrd, uint64_t va_start, uint64_t pa_start, size_t size, uint64_t prot) {
    uint64_t pte_flags = prot_to_flag(prot);
    uint64_t len = ALIGN(size, PAGE_SIZE);
    for (uint64_t offset = 0; offset < len; offset += PAGE_SIZE) {
        map_page(thrd, va_start + offset, pa_start + offset, pte_flags);
    }

    vma_add(thrd, va_start, va_start + len, prot, 0);
}

void *vm_mmap(sched_task_t *thrd, uint64_t addr, size_t len, int prot, int flags, int fd, int file_offset) {
    if (!(flags & MAP_ANONYMOUS)) return (void*)-1;      // Validate flags (only MAP_ANONYMOUS supported)
    else if (len <= 0) return (void*)-1;

    len = ALIGN(len, PAGE_SIZE);                  // Ceiling alignment
   
    uint64_t va = addr;
    uint64_t hint = va & ~(PAGE_SIZE - 1);        // Flooring alignment

    va = vma_find_free(thrd, hint, len);
    if (!va) return (void*)-1;

    if (flags & MAP_POPULATE) {
        void *block = kmalloc(len);
        vm_map_pages(thrd, va, VA_TO_PA(block), len, prot);
    } else {
        vma_add(thrd, va, va + len, prot, 0);
    }

    return (void*)va;
}

int vm_fault_handle(uint64_t va, esr_el1_t esr) {
    sched_task_t *curr = sched_get_current();

    if (FSC(esr.iss) == ABORT_ISS_TRANS_FAULT_L0 || FSC(esr.iss) == ABORT_ISS_TRANS_FAULT_L1 ||
        FSC(esr.iss) == ABORT_ISS_TRANS_FAULT_L2 || FSC(esr.iss) == ABORT_ISS_TRANS_FAULT_L3) {   
        
        vm_area_t *pos, *tmp, *vma = NULL;
        list_for_each_entry_safe(pos, tmp, &curr->vm_area_queue, list) {
            if (va >= pos->start && va < pos->end) {
                vma = pos;
                break;
            }
        }
    
        if (vma) {
            uint64_t va_aligned = va & ~(PAGE_SIZE - 1);
            uint64_t page;
            if (vma->file) {
                uint64_t offset = (va - vma->start) & ~(PAGE_SIZE - 1);
                uint64_t addr = vma->file + offset;
                page = addr;
            } else {
                page = (uint64_t)page_alloc(0);
            }
            
            uint64_t pte_flags = prot_to_flag(vma->prot);
            map_page(curr, va_aligned, page, pte_flags);

            uart_printf("Translation fault: %p\n", va);
            return 0;
        }
    }

    uart_printf("Segmentation fault\n");
    return -1;
}

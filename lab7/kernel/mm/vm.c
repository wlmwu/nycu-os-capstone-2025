#include "vm.h"
#include "utils.h"
#include "buddy.h"
#include "mini_uart.h"
#include "proc.h"
#include "slab.h"
#include "list.h"
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

static vm_area_t* vma_get(sched_task_t *thrd, uint64_t va) {
    vm_area_t *pos, *tmp, *vma = NULL;
    list_for_each_entry_safe(pos, tmp, &thrd->vm_area_queue, list) {
        if (va >= pos->start && va < pos->end) {
            vma = pos;
            break;
        }
    }
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

static uint64_t* walk(sched_task_t *thrd, uint64_t va) {
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
        table = (uint64_t*)PA_TO_VA((uint64_t)(table[idx] & PAGE_MASK));
    }

    uint64_t idx = PAGE_IDX(3, va);                 // PTE(3)
    return &table[idx];
}

void vm_map_pages(sched_task_t *thrd, uint64_t va_start, uint64_t pa_start, size_t size, uint64_t prot) {
    uint64_t pte_flags = prot_to_flag(prot);
    uint64_t len = ALIGN(size, PAGE_SIZE);
    for (uint64_t offset = 0; offset < len; offset += PAGE_SIZE) {
        uint64_t *entry = walk(thrd, va_start + offset);
        uint64_t page = pa_start + offset;
        *entry = page | PD_AF | PD_MAIR_NORMAL_NOCACHE | PD_TYPE_PAGE | pte_flags;
    }

    vma_add(thrd, va_start, va_start + len, prot, 0);
}

void *vm_mmap(sched_task_t *thrd, uint64_t addr, size_t len, int prot, int flags, int fd, int file_offset) {
    if (!(flags & MAP_ANONYMOUS)) return (void*)-1;      // Validate flags (only MAP_ANONYMOUS supported)
    else if (len <= 0) return (void*)-1;

    len = ALIGN(len, PAGE_SIZE);                  // Ceiling alignment
   
    uint64_t va = addr;
    uint64_t hint = va & PAGE_MASK;               // Flooring alignment

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

    if (FSC(esr.iss) == ABORT_ISS_FSC_TRANS_L0 || FSC(esr.iss) == ABORT_ISS_FSC_TRANS_L1 ||
        FSC(esr.iss) == ABORT_ISS_FSC_TRANS_L2 || FSC(esr.iss) == ABORT_ISS_FSC_TRANS_L3) {   
        vm_area_t *vma = vma_get(curr, va);
        if (vma) {
            uint64_t va_aligned = va & PAGE_MASK;
            uint64_t page = (uint64_t)page_alloc(0);
            if (vma->file) {
                uint64_t offset = (va - vma->start) & PAGE_MASK;
                uint64_t addr = vma->file + offset;
                memcpy((void*)PA_TO_VA(page), (void*)PA_TO_VA(addr), PAGE_SIZE);
            }
            
            uint64_t pte_flags = prot_to_flag(vma->prot);
            uint64_t *entry = walk(curr, va_aligned);
            *entry = page | PD_AF | PD_MAIR_NORMAL_NOCACHE | PD_TYPE_PAGE | pte_flags;

            tlb_flush_page(va);

            uart_printf("Translation fault: %p\n", va);
            return 0;
        }
    } else if (WNR(esr.iss) == ABORT_ISS_WNR_WRITE) {
        vm_area_t *vma = vma_get(curr, va);
        if (vma && vma->prot & PROT_WRITE) {
            uint64_t *entry = walk(curr, va);
            uint64_t old_page = *entry & PAGE_MASK;
            if (page_refcount_update(old_page, 0) > 1) {
                uint64_t new_page = (uint64_t)page_alloc(0);
                memcpy((void*)PA_TO_VA(new_page), (void*)PA_TO_VA(old_page), PAGE_SIZE);
                *entry &= ~PAGE_MASK;
                *entry |= new_page;
                page_refcount_update(old_page, -1);
            }
            *entry &= ~PD_AP_RO_EL0;
            *entry |= PD_AP_RW_EL0;

            tlb_flush_all();

            uart_printf("Writing fault: %p\n", va);
            return 0;
        }
    }

    uart_printf("Segmentation fault\n");
    return -1;
}

static void copy_tables(int pgidx, uint64_t dst_table, uint64_t src_table, uint64_t level) {
    dst_table = PA_TO_VA(dst_table);
    src_table = PA_TO_VA(src_table);

    if (level > 3 || level < 0) return;
    if (!src_table) return;

    if (level == 3) {       // PTE
        if (((uint64_t*)src_table)[pgidx] & PD_TYPE_PAGE) {
            if (((uint64_t*)src_table)[pgidx] & PD_AP_RW_EL0) {
                ((uint64_t*)src_table)[pgidx] &= ~PD_AP_RW_EL0;
                ((uint64_t*)src_table)[pgidx] |= PD_AP_RO_EL0;
            }
            ((uint64_t*)dst_table)[pgidx] = ((uint64_t*)src_table)[pgidx];
            page_refcount_update(((uint64_t*)dst_table)[pgidx] & PAGE_MASK, +1);
        }
    } else {                // PGD, PUD, PMD
        if (((uint64_t*)src_table)[pgidx] & PD_TYPE_TABLE) {
            if (!((uint64_t*)dst_table)[pgidx]) {
                uint64_t table = (uint64_t)page_alloc(0);
                ((uint64_t*)dst_table)[pgidx] = table | PD_TYPE_TABLE;
                memset((void*)PA_TO_VA(table), 0, PAGE_SIZE);
            }
            dst_table = ((uint64_t*)dst_table)[pgidx] & PAGE_MASK;
            src_table = ((uint64_t*)src_table)[pgidx] & PAGE_MASK;
            for (int i = 0; i < TABLE_SIZE; ++i) {
                copy_tables(i, dst_table, src_table, level + 1);
            }
        }
    }
}

void vm_copy(sched_task_t *dst, sched_task_t *src) {
    vm_area_t *vma;
    list_for_each_entry(vma, &src->vm_area_queue, list) {
        vma_add(dst, vma->start, vma->end, vma->prot, vma->file);
    }

    for (int i = 0; i < TABLE_SIZE; ++i) {
        copy_tables(i, dst->pgd, src->pgd, 0);
    }
}

static void free_tables(uint64_t table, uint64_t level) {
    if (!table) return;
    
    // Page frame
    if (level == 4) {   
        page_refcount_update(table, -1);
        return;
    }
    
    // PGD, PUD, PMD, PTE
    for (int i = 0; i < TABLE_SIZE; ++i) {
        uint64_t nxt_table = ((uint64_t*)PA_TO_VA(table))[i] & PAGE_MASK;
        free_tables(nxt_table, level + 1);
        ((uint64_t*)PA_TO_VA(table))[i] = 0ULL;
    }
    if (level != 0) {
        page_refcount_update(table, -1);
    }
} 

void vm_release(sched_task_t *thrd) {
    // Free VMAs
    while (thrd->vm_area_queue.next != &thrd->vm_area_queue) {
        list_del(thrd->vm_area_queue.next);
    }
    INIT_LIST_HEAD(&thrd->vm_area_queue);

    // Free page tables
    free_tables(thrd->pgd, 0);

    // Flush TLB
    tlb_flush_all();
}
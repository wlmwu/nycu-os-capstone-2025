#include "vm.h"
#include "utils.h"
#include "buddy.h"
#include "mini_uart.h"

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
    table[idx] = page | PD_AF | PD_AP_USER | PD_MAIR_NORMAL_NOCACHE | PD_TYPE_PAGE| flag;
}

void vm_map_pages(sched_task_t *thrd, uint64_t va_start, uint64_t pa_start, size_t size, uint64_t flag) {
    int64_t sz = size;
    while (sz > 0) {
        map_page(thrd, va_start, pa_start, flag);
        va_start += PAGE_SIZE;
        pa_start += PAGE_SIZE;
        sz -= PAGE_SIZE;
    }
}
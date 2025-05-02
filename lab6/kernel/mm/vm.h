#ifndef VM_H_
#define VM_H_

#include "sched.h"
#include "mmu.h"
#include <stdint.h>

/**
 * Extracts the page table index at `level` from the virtual address `va`.
 *
 * @param level: The level of the page table being indexed (e.g., 0 for PGD, 1 for PUD, etc.)
 * @param va: The virtual address for which the page table index is being calculated
 *
 * @return The index into the page table at the specified level.
 */
#define PAGE_IDX(level, va)  (((va) >> (PAGE_SHIFT + TABLE_SHIFT * (3 - (level)))) & 0x1FF)


/**
 * @brief Maps a contiguous range of virtual addresses to a contiguous range of physical addresses.
 *
 * This function iteratively calls `map_page` to establish mappings for a sequence of virtual
 * pages starting at `va_start` to corresponding physical pages starting at `pa_start`. The
 * number of pages to map is determined by the `size` parameter. The same set of flags (`flag`)
 * is applied to all the created page table entries.
 *
 * @param task The task whose address space will be modified.
 * @param va_start The starting virtual address of the range to be mapped.
 * @param pa_start The starting physical address of the range to which the virtual addresses will be mapped.
 * @param size The total size of the memory region to be mapped, in bytes. This value is expected to be a multiple of the page size.
 * @param flag Flags to be set in all the created page table entries.
 */
void vm_map_pages(sched_task_t *thrd, uint64_t va_start, uint64_t pa_start, size_t size, uint64_t flag);

#endif // VM_H_

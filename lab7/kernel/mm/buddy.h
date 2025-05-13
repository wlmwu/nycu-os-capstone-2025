#ifndef BUDDY_H_
#define BUDDY_H_

#include "mmu.h"
#include <stdint.h>

#define MAX_ORDER 11
#define MEM_START 0x0
#define MEM_END 0x3C000000
#define MAX_PAGES ((MEM_END - MEM_START) / PAGE_SIZE)

/**
 * Calculate the buddy block's Page Frame Number (PFN) for a given block and order.
 * The buddy block is the sibling block in the buddy system's binary tree structure.
 * The formula works by flipping the bit at the position corresponding to the block's order.
 * Flipping this bit is mathematically equivalent to adding or subtracting the block size
 * (2^order * PAGE_SIZE) from the current block's PFN.
 * 
 * @param pfn: Page Frame Number (PFN) of the current block
 * @param order: Order of the block (size = 2^order * PAGE_SIZE)
 *
 * @return the PFN of the buddy block.
 */
#define BUDDY(pfn, order) ((pfn) ^ (1 << (order)))

/**
 * Check if a block's Page Frame Number (PFN) is invalid for a given order.
 * A block is considered invalid if it is not aligned to its order.
 * This is determined by checking if the lower `order` bits of the PFN are non-zero.
 * 
 * @param pfn: Page Frame Number (PFN) of the block
 * @param order: Order of the block (size = 2^order * PAGE_SIZE)
 *
 * @return 1 if the block is invalid (not aligned), 0 otherwise.
 */
#define IS_BLOCK_INVALID(pfn, order) (pfn & ((1 << order) - 1))

void buddy_reserve(uintptr_t start, uintptr_t end);

void buddy_init();
void *page_alloc(int order);
void page_free(void *addr);

/**
 * @brief Updates the reference count for the page associated with the given virtual address.
 *
 * This function adjusts the reference count of the provided physical address (`pa`).
 * If decrementing the count results in a zero reference count, the associated page is 
 * freed automatically.
 *
 * @param pa The physical address whose associated page reference count is to be updated.
 * @param increment The value to add to the reference count. Typically, this should be
 * 1 to increment the count, -1 to decrement it, or 0 to retrieve the current count 
 * without modification.
 * @return The updated reference count of the page on success. Returns -1 if `pa` is 
 * not valid (cannot be translated to a valid PFN or the corresponding memory block is 
 * marked as free).
 * 
 */
uint32_t page_refcount_update(uint64_t pa, int8_t increment);

#endif // BUDDY_H_
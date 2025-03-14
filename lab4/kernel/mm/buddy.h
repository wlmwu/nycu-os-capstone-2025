#ifndef BUDDY_H_
#define BUDDY_H_

#define MAX_ORDER 11
#define MEM_START 0x10000000
#define MEM_END 0x20000000
#define PAGE_SIZE 4096
#define MAX_PAGES ((MEM_END - MEM_START) / PAGE_SIZE)

/*
Calculate the size of the bitmap required to manage page allocations for different orders.
1. First, calculate the total number of pages (`MAX_PAGES`) between `MEM_START` and `MEM_END`, based on `PAGE_SIZE`:
   `MAX_PAGES = (MEM_END - MEM_START) / PAGE_SIZE`

2. For each order, the number of blocks (`num_blocks`) that need to be tracked in the bitmap is:
   `num_blocks = MAX_PAGES / (2^order)`

3. The bitmap for each order needs one bit per block, so the size of the bitmap (in bytes) for a given order is calculated as:
   `MASK_ORDER_SIZE(order) = (num_blocks + 7) / 8`
   This formula ensures that the number of bits required to track the blocks is rounded up to the next byte.

4. To calculate the total bitmap size, sum the sizes of the bitmaps for all orders from `0` to `MAX_ORDER - 1`:
   `MASK_SIZE = SUM(MASK_ORDER_SIZE(order))` for all orders from `0` to `MAX_ORDER - 1`
*/
#define MASK_SIZE 16376

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



void buddy_init();
void *pfn_alloc(int order);
void pfn_free(void *addr, int order);

#endif // BUDDY_H_
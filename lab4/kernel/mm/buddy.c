#include "buddy.h"
#include "list.h"
#include "utils.h"
#include <stdint.h>
#include <stddef.h>

#include "mini_uart.h"

// Allocate in O(1)
static struct list_head free_lists[MAX_ORDER];

// Coalesce in O(1)
static unsigned char bitmap_buffer[MASK_SIZE];
// 1 means free
static unsigned char *free_masks[MAX_ORDER]; 


static void *pfn_to_addr(unsigned long pfn) {
    return (void*)((pfn * PAGE_SIZE) + MEM_START);
}
static unsigned long addr_to_pfn(void *addr) {
    return ((uintptr_t)addr - MEM_START) / PAGE_SIZE;
}

static void mask_set_free(int order, unsigned long pfn) {
    int index = pfn >> order;
    free_masks[order][index / 8] |= (1 << (index % 8));
}

static void mask_set_unfree(int order, unsigned long pfn) {
    int index = pfn >> order;
    free_masks[order][index / 8] &= ~(1 << (index % 8));
}

static int mask_is_free(int order, unsigned long pfn) {
    if (pfn >= MAX_PAGES) return 1;
    int index = pfn >> order;
    return (free_masks[order][index / 8] & (1 << (index % 8))) != 0;
}

static void free_add(int order, unsigned long pfn) {
    struct list_head *block = pfn_to_addr(pfn);
    INIT_LIST_HEAD(block);
    list_add_tail(block, &free_lists[order]);
    mask_set_free(order, pfn);
    uart_printf("[+] Add PFN %u to %u. Current list head: %p\n", pfn, order, free_lists[order].next);
}

static void free_remove(int order, unsigned long pfn) {
    struct list_head *block = pfn_to_addr(pfn);
    list_del(block);
    mask_set_unfree(order, pfn);
    uart_printf(" [-] Remove PFN %u from %u. Current list head: %p\n", pfn, order, free_lists[order].next);
}


void buddy_init() {
    for (int i = 0; i < MAX_ORDER; ++i) {
        INIT_LIST_HEAD(&free_lists[i]);
    }
    
    unsigned char *current = bitmap_buffer;
    for (int order = 0; order < MAX_ORDER; ++order) {
        unsigned int num_blocks = MAX_PAGES >> order;
        free_masks[order] = current;
        memset(current, 0, (num_blocks + 7) / 8);
        current += (num_blocks + 7) / 8;
    }
    free_add(MAX_ORDER - 1, 0);
}

void *pfn_alloc(int order) {
    if (order < 0 || order >= MAX_ORDER) {
        return NULL;
    }

    int current_order = order;
    while (current_order < MAX_ORDER && list_empty(&free_lists[current_order])) {
        ++current_order;
    }
    if (current_order >= MAX_ORDER) {
        return NULL;
    }

    for ( ; current_order > order; --current_order) {
        struct list_head *block = free_lists[current_order].next;
        unsigned long block_pfn = addr_to_pfn(block);
        free_remove(current_order, block_pfn);

        unsigned long buddy_pfn = BUDDY(block_pfn, current_order - 1);
        struct list_head *buddy = pfn_to_addr(buddy_pfn);

        free_add(current_order - 1, block_pfn);
        free_add(current_order - 1, buddy_pfn);
    }
    
    // current_order == order
    struct list_head *block = free_lists[current_order].next;
    unsigned long block_pfn = addr_to_pfn(block);
    free_remove(current_order, block_pfn);
    
    uart_printf("\033[0;33mAllocate Page at order %d, pfn %d\033[0m\n", current_order, block_pfn, block);

    return block;
}

void pfn_free(void *addr, int order) {
    if (order < 0 || order >= MAX_ORDER) {
        return;
    }
    
    unsigned long block_pfn = addr_to_pfn(addr);
    
    if (mask_is_free(order, block_pfn)) {
        return;
    }
    
    for ( ; order < MAX_ORDER; ++order) {
        unsigned long buddy_pfn = BUDDY(block_pfn, order);
        if (IS_BLOCK_INVALID(buddy_pfn, order) || !mask_is_free(order, buddy_pfn)) {
            break;
        }

        free_remove(order, buddy_pfn);
        block_pfn = block_pfn & buddy_pfn;      // Merge to lower PFN
    }

    free_add(order, block_pfn);
    
    uart_printf("\033[0;33mFree Page at order %d, pfn %d\033[0m\n", order, block_pfn);
}
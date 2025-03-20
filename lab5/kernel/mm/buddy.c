#include "buddy.h"
#include "list.h"
#include "utils.h"
#include <stdint.h>
#include <stddef.h>

#include "mini_uart.h"

// Allocate in O(1)
static struct list_head free_lists[MAX_ORDER];

// Coalesce in O(1)
static unsigned char *bitmap_buffer;
// 1 means free
static unsigned char *free_masks[MAX_ORDER]; 


static unsigned char reserved_bitmap[(MAX_PAGES + 7) / 8 * 8];

void buddy_reserve(uintptr_t start, uintptr_t end) {
    if (end < MEM_START || start > MEM_END) return;
    start = MAX(start, MEM_START);
    end = MIN(end, MEM_END);
    unsigned long start_pfn = (start - MEM_START) / PAGE_SIZE;
    unsigned long end_pfn = (end - MEM_START + PAGE_SIZE - 1) / PAGE_SIZE;
    // uart_printf("[x] Reserve address [%x, %x), PFN [%d, %d)\033[0m\n", start, end, start_pfn, end_pfn);

    for (unsigned long pfn = start_pfn; pfn < end_pfn; pfn++) {
        reserved_bitmap[pfn / 8] |= (1 << (pfn % 8));
    }
}

static int is_reserved(unsigned long pfn) {
    if (pfn >= MAX_PAGES) return 1;
    return (reserved_bitmap[pfn / 8] & (1 << (pfn % 8))) != 0;
}

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
    // uart_printf("[+] Add PFN %u to order %u. Current list head: %p\n", pfn, order, free_lists[order].next);
}

static void free_remove(int order, unsigned long pfn) {
    struct list_head *block = pfn_to_addr(pfn);
    list_del(block);
    mask_set_unfree(order, pfn);
    // uart_printf("[-] Remove PFN %u from order %u. Current list head: %p\n", pfn, order, free_lists[order].next);
}


void buddy_init() {
    for (int i = 0; i < MAX_ORDER; ++i) {
        INIT_LIST_HEAD(&free_lists[i]);
    }
    
    unsigned long bitmap_size = 0;
    for (int i = 0; i < MAX_ORDER; ++i) {
        unsigned long num_blocks = MAX_PAGES >> i;
        unsigned long order_make_size = (num_blocks + 7) / 8;
        bitmap_size += order_make_size;
    }
    bitmap_buffer = (unsigned char*)malloc(bitmap_size);
    unsigned char *current = bitmap_buffer;
    for (int order = 0; order < MAX_ORDER; ++order) {
        unsigned int num_blocks = MAX_PAGES >> order;
        free_masks[order] = current;
        memset(current, 0, (num_blocks + 7) / 8);
        current += (num_blocks + 7) / 8;
    }

    unsigned long current_pfn = 0;
    while (current_pfn < MAX_PAGES) {
        if (is_reserved(current_pfn)) {
            ++current_pfn;
            continue;
        }

        int order = 0;
        for (unsigned long i = current_pfn ; i < MAX_PAGES && order < MAX_ORDER; ++i) {
            if (is_reserved(i)) {
                break;
            }
            if (i - current_pfn + 1 == 1 << order) {
                ++order;
                if (IS_BLOCK_INVALID(current_pfn, order)) {
                    break;
                }
            }
        }
        --order;
        if (order >= 0) {
            free_add(order, current_pfn);
            current_pfn += 1 << order;
        } else {
            ++current_pfn;
        }
    }
}

void *page_alloc(int order) {
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
    
    // uart_printf("\033[0;33mAllocate Page at order %d, pfn %d\033[0m\n", current_order, block_pfn, block);

    return block;
}

void page_free(void *addr, int order) {
    if (order < 0 || order >= MAX_ORDER) {
        return;
    }
    
    unsigned long block_pfn = addr_to_pfn(addr);
    
    if (mask_is_free(order, block_pfn)) {
        return;
    }
    
    for ( ; order < MAX_ORDER - 1; ++order) {
        unsigned long buddy_pfn = BUDDY(block_pfn, order);
        if (IS_BLOCK_INVALID(buddy_pfn, order) || !mask_is_free(order, buddy_pfn)) {
            break;
        }

        free_remove(order, buddy_pfn);
        block_pfn = block_pfn & buddy_pfn;      // Merge to lower PFN
    }

    free_add(order, block_pfn);
    
    // uart_printf("\033[0;33mFree Page at order %d, pfn %d\033[0m\n", order, block_pfn);
}
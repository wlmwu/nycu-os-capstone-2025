#include "slab.h"
#include "buddy.h"
#include "mmu.h"
#include "irq.h"
#include "list.h"
#include <stddef.h>
#include <stdint.h>

#include "mini_uart.h"

#define MAX_CHUNK_SIZE 1024
#define NUM_POOLS 7     // `MAX_CHUNK_SIZE`, `MAX_CHUNK_SIZE/2`, ..., `MAX_CHUNK_SIZE/(2^6)`

typedef struct slab_header {
    size_t chunk_size;
    size_t free_list_size;
    struct list_head free_list;     // Linked all free chunks
    struct list_head list;          // Linked the slab with its corresponding pools[i] 
} slab_header_t;

static struct list_head pools[NUM_POOLS];   // Each pool represents the `list_head` of the slab headers with the same chunk size


static int pool_find(size_t size) {
    for (int i = 0; i < NUM_POOLS; ++i) {
        size_t chunk_size = MAX_CHUNK_SIZE >> (NUM_POOLS - i - 1);
        if (chunk_size >= size) {
            return i;
        }
    }
    return -1;
}

static int order_find(size_t size) {
    size_t num_page = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    int order = 0;
    while ((1 << order) < num_page && order < MAX_ORDER) {
        order++;
    }
    return (order < MAX_ORDER) ? order : -1;
}

void slab_init() {
    for (int i = 0; i < NUM_POOLS; ++i) {
        INIT_LIST_HEAD(&pools[i]);
    }
}

void *kmalloc(size_t size) {
    irq_lock_t lock;
    irq_lock(&lock);
    int pool_idx = pool_find(size);
    if (pool_idx < 0) {                     // Allocate a block if size is too large
        int order = order_find(size);
        irq_unlock(&lock);
        return (void*)PA_TO_VA(page_alloc(order));
    }

    if (list_empty(&pools[pool_idx])) {
        void *page = page_alloc(0);
        if (!page) {
            irq_unlock(&lock);
            return NULL;
        }
        page = (void*)PA_TO_VA(page);
        
        slab_header_t *slab_header = page;
        slab_header->chunk_size =  MAX_CHUNK_SIZE >> (NUM_POOLS - pool_idx - 1);
        INIT_LIST_HEAD(&slab_header->free_list);
        slab_header->free_list_size = 0;
        list_add_tail(&slab_header->list, &pools[pool_idx]);

        char *chunk_start = (char*)page + sizeof(slab_header_t);
        size_t total_chunks = (PAGE_SIZE - sizeof(slab_header_t)) / slab_header->chunk_size;
        for (size_t i = 0; i < total_chunks; ++i) {
            struct list_head *chunk = (struct list_head*)(chunk_start + (i * slab_header->chunk_size));
            list_add_tail(chunk, &slab_header->free_list);
            ++slab_header->free_list_size;
        }
    }

    slab_header_t *slab_header = list_entry(pools[pool_idx].next, slab_header_t, list);
    struct list_head *chunk = slab_header->free_list.next;
    list_del(chunk);
    --slab_header->free_list_size;

    if (slab_header->free_list_size == 0) {
        list_del(&slab_header->list);
    }

    // uart_printf("\033[0;33m[Chunk]\tAllocate %p at chunk size %u\033[0m\n", chunk, slab_header->chunk_size);

    irq_unlock(&lock);
    return chunk;
}

void kfree(void *ptr) {
    if (!VA_TO_PA(ptr)) return;
    irq_lock_t lock;
    irq_lock(&lock);

    uintptr_t slab_start = ((uintptr_t)ptr) & ~(PAGE_SIZE - 1);      // Mask lower bits
    if (slab_start == ((uintptr_t)ptr)) {           // `ptr` is the start address of a page
        page_free((void*)VA_TO_PA(ptr));
        irq_unlock(&lock);
        return;
    }

    slab_header_t *slab_header = (slab_header_t *)slab_start;
    struct list_head *chunk = ptr;

    list_add_tail(chunk, &slab_header->free_list);
    ++slab_header->free_list_size;

    int pool_idx = pool_find(slab_header->chunk_size);
    if (pool_idx < 0) {
        irq_unlock(&lock);
        return;
    }

    // uart_printf("\033[0;33m[Chunk]\tFree %p at chunk size %u\033[0m\n", chunk, slab_header->chunk_size);

    size_t total_chunks = (PAGE_SIZE - sizeof(slab_header_t)) / slab_header->chunk_size;
    if (slab_header->free_list_size == 1) {
        list_add_tail(&slab_header->list, &pools[pool_idx]);
    } else if (slab_header->free_list_size == total_chunks) {
        list_del(&slab_header->list);
        page_free((void*)VA_TO_PA(slab_header));
    }

    irq_unlock(&lock);
}
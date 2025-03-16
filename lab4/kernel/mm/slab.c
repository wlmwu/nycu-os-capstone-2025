#include "slab.h"
#include "buddy.h"
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


int pool_find(size_t size) {
    for (int i = 0; i < NUM_POOLS; ++i) {
        size_t chunk_size = MAX_CHUNK_SIZE >> (NUM_POOLS - i - 1);
        if (chunk_size >= size) {
            return i;
        }
    }
    return -1;
}


void slab_init() {
    for (int i = 0; i < NUM_POOLS; ++i) {
        INIT_LIST_HEAD(&pools[i]);
    }
}

void *kmalloc(size_t size) {
    int pool_idx = pool_find(size);
    if (pool_idx < 0) {                     // Allocate a page if size is too large
        // return pfn_alloc(((size + PAGE_SIZE - 1) / PAGE_SIZE) - 1);
        return page_alloc(0);
    }

    if (list_empty(&pools[pool_idx])) {
        void *page = page_alloc(0);
        if (!page) return NULL;


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

    // uart_printf("\033[0;33mAllocate chunk at %p, page %p, chunk size %u\033[0m\n", chunk, slab_header, slab_header->chunk_size);

    return chunk;
}

void kfree(void *ptr) {
    if (!ptr) return;

    uintptr_t slab_start = ((uintptr_t)ptr) & ~(PAGE_SIZE - 1);      // Mask lower bits
    if (slab_start == ((uintptr_t)ptr)) {           // ptr is the start address of a page
        page_free(ptr, 0);
        return;
    }

    slab_header_t *slab_header = (slab_header_t *)slab_start;
    struct list_head *chunk = ptr;

    list_add_tail(chunk, &slab_header->free_list);
    ++slab_header->free_list_size;

    int pool_idx = pool_find(slab_header->chunk_size);
    if (pool_idx < 0) return;

    // uart_printf("\033[0;33mFree chunk at %p, page %p, chunk size %u\033[0m\n", chunk, slab_header, slab_header->chunk_size);

    size_t total_chunks = (PAGE_SIZE - sizeof(slab_header_t)) / slab_header->chunk_size;
    if (slab_header->free_list_size == 1) {
        list_add_tail(&slab_header->list, &pools[pool_idx]);
    } else if (slab_header->free_list_size == total_chunks) {
        list_del(&slab_header->list);
        page_free(slab_header, 0);
    }
}
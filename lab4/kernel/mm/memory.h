#ifndef MEMORY_H_
#define MEMORY_H_

#include "buddy.h"
#include "slab.h"

#define SPIN_TABLE_START 0x0000
#define SPIN_TABLE_END 0x1000

extern unsigned long _start;
extern unsigned long _end;

/**
 * @brief Reserves a range of memory addresses before memory initialization.
 *
 * This function reserves a region of memory in the buddy system. It is important
 * to call this function **before** initializing the memory management system using
 * `memory_init()` or similar functions. The reservation ensures that specific memory
 * ranges (e.g., kernel image, device tree, etc.) are not used by the memory allocator
 * during the boot process.
 *
 * @param start The start address (inclusive) of the memory region to reserve.
 * @param end The end address (exclusive) of the memory region to reserve.
 *
 * @note This function must be called **before** the memory system is initialized
 *       to avoid interference with memory management structures.
 */
static inline void memory_reserve(uintptr_t start, uintptr_t end) {
    buddy_reserve(start, end);
}

/**
 * @brief Initializes the memory management system.
 * 
 * This function initializes the memory management system, setting up the buddy system,
 * memory pools, and other structures required for memory allocation. It should be 
 * called **after** memory reservations have been made using `memory_reserve()` to ensure that
 * reserved memory regions are protected and not used by the memory allocator.
 *
 * @note It is important to note that all **`memory_reserve()`** should be called **before**
 *       calling this function.
 */
static inline void memory_init() {
    memory_reserve((uintptr_t)(SPIN_TABLE_START), (uintptr_t)(0x1000));
    memory_reserve((uintptr_t)(&_start), (uintptr_t)(&_end));

    buddy_init();
    slab_init();
}

#endif // MEMORY_H_
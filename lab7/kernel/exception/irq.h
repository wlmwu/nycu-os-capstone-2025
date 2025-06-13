#ifndef IRQ_H_
#define IRQ_H_

#include "reg_base.h"

#define AUX_INT (1 << 29)
#define CORE0_INTERRUPT_SOURCE (VA_BASE + 0x40000060)
#define INTERRUPT_SOURCE_GPU (1 << 8)
#define INTERRUPT_SOURCE_CNTPNSIRQ (1 << 1)

#include <stdint.h>
#include <stddef.h>

typedef void (*irq_task_fn_t)(void *args);
typedef struct irq_task {
    irq_task_fn_t handle_fn;
    void *args;
    uint64_t priority;
} irq_task_t;

irq_task_t* irq_task_construct(irq_task_fn_t fn, void *args, size_t argsize, uint64_t priority);
void irq_task_destruct(irq_task_t *task);

void irq_enable();
void irq_disable();
void irq_handle();

void irq_init();

void irq_handle_pending_tasks();

#include "utils.h"

typedef uint64_t irq_lock_t; 

static inline void irq_lock(irq_lock_t *lock) {
    *lock = READ_SYSREG(daif);
    WRITE_SYSREG((*lock | (0x2 << 6)), daif);
}

static inline void irq_unlock(irq_lock_t *lock) {
    WRITE_SYSREG(*lock, daif);
}

#endif // IRQ_H_

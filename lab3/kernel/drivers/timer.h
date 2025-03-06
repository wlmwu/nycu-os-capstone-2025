#ifndef TIMER_H_
#define TIMER_H_

#define CORE0_TIMERS_INTERRUPT_CONTROL 0x40000040

#include "list.h"
#include <stdint.h>
#include <stddef.h>

typedef void (*timer_callback_fn_t)(void *args);
typedef struct timer_event {
    timer_callback_fn_t callback;
    void *args;
    uint64_t expire_tick;
    struct list_head node;  // Linked list node
} timer_event_t;

void timer_irq_enable();
void timer_irq_disable();
void timer_irq_handle();

void timer_set_tick(uint64_t tick);

uint64_t timer_get_current_tick();
uint64_t timer_second_to_tick(uint64_t second);
uint64_t timer_tick_to_second(uint64_t tick);


void timer_add_event(timer_callback_fn_t fn, void *args, size_t argsize, uint64_t duration);


#endif // TIMER_H_
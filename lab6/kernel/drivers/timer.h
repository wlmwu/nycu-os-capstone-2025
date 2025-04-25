#ifndef TIMER_H_
#define TIMER_H_

#define CORE0_TIMERS_INTERRUPT_CONTROL 0x40000040

#include <stdint.h>
#include <stddef.h>

typedef void (*timer_callback_fn_t)(void *args);
typedef struct timer_event {
    timer_callback_fn_t callback;
    void *args;
    uint64_t expire_tick;
} timer_event_t;

timer_event_t* timer_event_construct(timer_callback_fn_t fn, void *args, size_t argsize, uint64_t expire_tick);
void timer_event_destruct(timer_event_t *event);

void timer_init();

void timer_irq_enable();
void timer_irq_disable();
void timer_irq_handle();

void timer_set_tick(uint64_t tick);

uint64_t timer_get_current_tick();
uint64_t timer_get_current_freq();
uint64_t timer_second_to_tick(uint64_t second);
uint64_t timer_tick_to_second(uint64_t tick);

/**
 * @brief Adds a new timer event to the timer event queue.
 *
 * @param fn        The callback function to be executed when the timer event expires.
 * @param args      A pointer to the arguments to be passed to the callback function.
 * @param argsize   The size of the arguments in bytes.
 * @param duration  The duration (in ticks) until the timer event expires.
 */
void timer_add_event(timer_callback_fn_t fn, void *args, size_t argsize, uint64_t duration);


#endif // TIMER_H_
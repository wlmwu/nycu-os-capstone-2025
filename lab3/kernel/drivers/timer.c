#include "timer.h"
#include "mini_uart.h"
#include "utils.h"
#include "priority_queue.h"
#include <stdbool.h>

static priority_queue_t *timer_event_queue;

static bool timer_event_comp(void *newn, void *n) {
    return ((timer_event_t*)newn)->expire_tick < ((timer_event_t*)n)->expire_tick;
}

void timer_init() {
    timer_event_queue = (priority_queue_t*)malloc(sizeof(priority_queue_t));
    pq_init(timer_event_queue, timer_event_comp);
}

void timer_set_tick(uint64_t tick) {
    asm volatile (
        "msr cntp_cval_el0, %[tick]        \n"      //cntp_cval_el0: absolute timer
        :: [tick] "r" (tick)
    );
}

void timer_irq_enable() {
    asm volatile(
        "mov    x1, %[ctrl_addr]           \n"      // 0x40000040: Core 0 Timers interrupt control
        "mov    x2, #2                     \n"
        "str    w2, [x1]                   \n"      // w2: lower 32 bits of x2. Set b'10 to address 0x40000040 => nCNTPNSIRQ IRQ enabled.
        :
        : [ctrl_addr] "r" (CORE0_TIMERS_INTERRUPT_CONTROL)
    );
}

void timer_irq_disable() {
    *((volatile unsigned int*)(CORE0_TIMERS_INTERRUPT_CONTROL)) = 0;
}

void timer_irq_handle() {
    timer_irq_disable();

    uint64_t current_tick = timer_get_current_tick();
    while (!pq_empty(timer_event_queue)) {
        timer_event_t *event = (timer_event_t*)pq_top(timer_event_queue);
        if (event->expire_tick > current_tick) {
            break;
        } else {
            pq_pop(timer_event_queue);
            event->callback(event->args);
            timer_event_destruct(event);
        }
    }

    if (!pq_empty(timer_event_queue)) {
        timer_event_t *event = (timer_event_t*)pq_top(timer_event_queue);
        timer_set_tick(event->expire_tick);
        timer_irq_enable();
    }
}

uint64_t timer_get_current_tick() {
    uint64_t cntpct;
    asm volatile("mrs %[pct], cntpct_el0\n\t" : [pct] "=r"(cntpct));    // Get the timer's current count (total number of ticks since system boot)
    return cntpct;
}

uint64_t timer_second_to_tick(uint64_t second) {
    uint64_t cntfrq;
    asm volatile("mrs %[frq], cntfrq_el0" : [frq] "=r"(cntfrq) );       // Read the timer frequency
    uint64_t timeout_value = cntfrq * second;  
    return timeout_value;
}

uint64_t timer_tick_to_second(uint64_t tick) {
    uint64_t cntfrq;
    asm volatile("mrs %[frq], cntfrq_el0" : [frq] "=r"(cntfrq) );       // Read the timer frequency
    return tick/cntfrq;
}

timer_event_t* timer_event_construct(timer_callback_fn_t fn, void *args, size_t argsize, uint64_t expire_tick) {
    timer_event_t *event = (timer_event_t*)malloc(sizeof(timer_event_t));
    event->callback = fn;
    event->expire_tick = expire_tick;
    if (args && argsize > 0) {
        event->args = malloc(argsize);
        if (!event->args) {
            uart_printf("Memory allocation for args failed\n");
            free(event);
            return NULL;
        }
        memcpy(event->args, args, argsize);
    } else {
        event->args = NULL;
    }
    return event;
}

void timer_event_destruct(timer_event_t *event) {
    free(event->args);
    free(event);
}

void timer_add_event(timer_callback_fn_t fn, void *args, size_t argsize, uint64_t duration) {
    uint64_t expire_tick = timer_second_to_tick(duration) + timer_get_current_tick();
    timer_event_t *event = timer_event_construct(fn, args, argsize, expire_tick);

    pq_push(timer_event_queue, (void*)event);

    uart_printf("Set event at %u sec, currently at %u sec.\n", timer_tick_to_second(event->expire_tick), timer_tick_to_second(timer_get_current_tick()));

    if (pq_top(timer_event_queue) == (void*)event) {
        timer_set_tick(event->expire_tick);
        timer_irq_enable();
    }
    
}
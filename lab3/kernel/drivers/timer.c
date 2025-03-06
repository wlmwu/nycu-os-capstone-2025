#include "timer.h"
#include "mini_uart.h"
#include "utils.h"

static LIST_HEAD(timer_event_list);

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

    timer_event_t *event, *nxt;
    // Used list_for_each_entry without _safe, deleting event inside the loop would cause 
    // invalid memory access when trying to access event->list.next in the next iteration.
    list_for_each_entry_safe(event, nxt, &timer_event_list, node) {
        if (event->expire_tick > current_tick) {
            break;
        }
        event->callback(event->args);
        list_del(&event->node);
        free(event->args);
        free(event);
    }

    if (!list_empty(&timer_event_list)) {
        event = list_entry(timer_event_list.next, timer_event_t, node);
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

void timer_add_event(timer_callback_fn_t fn, void *args, size_t argsize, uint64_t duration) {
    timer_event_t *event = (timer_event_t*)malloc(sizeof(timer_event_t));
    INIT_LIST_HEAD(&event->node);
    event->callback = fn;
    event->expire_tick = timer_second_to_tick(duration) + timer_get_current_tick();
    if (args && argsize > 0) {
        event->args = malloc(argsize);
        if (!event->args) {
            uart_printf("Memory allocation for args failed\n");
            free(event);
            return;
        }
        memcpy(event->args, args, argsize);
    } else {
        event->args = NULL;
    }

    timer_event_t *pos;
    list_for_each_entry(pos, &timer_event_list, node) {
        if (event->expire_tick < pos->expire_tick) {
            list_add_tail(&event->node, &pos->node);
            break;
        }
    }

    // pos == head : loop execute until the end
    if (&(pos->node) == &timer_event_list) {
        list_add_tail(&event->node, &timer_event_list);
    }

    uart_printf("Set event at %u secs, currently at %u secs.\n", timer_tick_to_second(event->expire_tick), timer_tick_to_second(timer_get_current_tick()));
    
    if (event == list_entry(timer_event_list.next, timer_event_t, node)) {
        timer_set_tick(event->expire_tick);
        timer_irq_enable();
    }
    
}
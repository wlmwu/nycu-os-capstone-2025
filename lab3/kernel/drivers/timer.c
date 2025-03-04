#include "timer.h"
#include "mini_uart.h"

void timer_set_time(unsigned int second) {
    unsigned long cntpct, cntfrq;
    asm volatile(
        "mrs %[pct], cntpct_el0\n\t"                // Get the timer's current count (total number of ticks since system boot)
        "mrs %[frq], cntfrq_el0"                    // Read the timer frequency
        : [pct] "=r"(cntpct), [frq] "=r"(cntfrq) 
    );

    unsigned long timeout_value = cntfrq * second;  
    asm volatile("msr    cntp_tval_el0, %[tval]" : : [tval] "r" (timeout_value));  // Set the next expiration time
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
    const int kTimeoutSeconds = 2;

    unsigned long cntpct, cntfrq;
    asm volatile(
        "mrs %[pct], cntpct_el0\n\t"                // Get the timer's current count (total number of ticks since system boot)
        "mrs %[frq], cntfrq_el0"                    // Read the timer frequency
        : [pct] "=r"(cntpct), [frq] "=r"(cntfrq) 
    );

    unsigned int current_time = cntpct / cntfrq;    // The time (in second) passed since system boot
    uart_printf("Time passed after booting: %u secs\n", current_time);

    timer_set_time(kTimeoutSeconds);
}

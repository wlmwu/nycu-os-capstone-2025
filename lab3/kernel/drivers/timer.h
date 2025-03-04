#ifndef TIMER_H_
#define TIMER_H_

#define CORE0_TIMERS_INTERRUPT_CONTROL 0x40000040

void timer_irq_enable();
void timer_irq_disable();
void timer_irq_handle();

void timer_set_time(unsigned int second);

#endif // TIMER_H_
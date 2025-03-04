#ifndef IRQ_H_
#define IRQ_H_

#define AUX_INT (1 << 29)
#define CORE0_INTERRUPT_SOURCE 0x40000060
#define INTERRUPT_SOURCE_GPU (1 << 8)
#define INTERRUPT_SOURCE_CNTPNSIRQ (1 << 1)

void irq_enable();
void irq_disable();
void irq_handle();

#endif // IRQ_H_

#ifndef EXCEOTION_H_
#define EXCEOTION_H_

void el1t_64_sync_handler(void *regs);
void el1t_64_irq_handler(void *regs);
void el1t_64_fiq_handler(void *regs);
void el1t_64_error_handler(void *regs);

void el1h_64_sync_handler(void *regs);
void el1h_64_irq_handler(void *regs);
void el1h_64_fiq_handler(void *regs);
void el1h_64_error_handler(void *regs);

void el0t_64_sync_handler(void *regs);
void el0t_64_irq_handler(void *regs);
void el0t_64_fiq_handler(void *regs);
void el0t_64_error_handler(void *regs);

void el0t_32_sync_handler(void *regs);
void el0t_32_irq_handler(void *regs);
void el0t_32_fiq_handler(void *regs);
void el0t_32_error_handler(void *regs);

#endif // EXCEOTION_H_
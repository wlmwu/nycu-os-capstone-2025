#ifndef EXCEOTION_H_
#define EXCEOTION_H_

#define EXCEPT_CLASS(esr) (esr >> 26)	// Extract exception class
#define EC_SVC 0x15						// SVC from AArch64
typedef struct trapframe {  // The layout here should be aligned with save_all/load_all in entry.S
	unsigned long x[30];        // x0-x29
	unsigned long lr;           // lr (x30)
	unsigned long sp;           // sp_el0
	unsigned long elr;          // elr_el1
	unsigned long spsr;         // spsr_el1
} trapframe_t;

void el1t_64_sync_handler(void *regs);
void el1t_64_irq_handler(void *regs);
void el1t_64_fiq_handler(void *regs);
void el1t_64_error_handler(void *regs);

void el1h_64_sync_handler(void *regs);
void el1h_64_irq_handler(trapframe_t *tf);
void el1h_64_fiq_handler(void *regs);
void el1h_64_error_handler(void *regs);

void el0t_64_sync_handler(trapframe_t *tf);
void el0t_64_irq_handler(trapframe_t *tf);
void el0t_64_fiq_handler(void *regs);
void el0t_64_error_handler(void *regs);

void el0t_32_sync_handler(void *regs);
void el0t_32_irq_handler(void *regs);
void el0t_32_fiq_handler(void *regs);
void el0t_32_error_handler(void *regs);

#endif // EXCEOTION_H_
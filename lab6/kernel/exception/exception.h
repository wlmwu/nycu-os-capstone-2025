#ifndef EXCEOTION_H_
#define EXCEOTION_H_

#include <stdint.h>

#define EC_SVC_64 					0x15			// SVC from AArch64
#define EC_INSTR_ABORT_LOWER_EL   	0x20			// Instruction Abort from a lower Exception level
#define EC_DATA_ABORT_LOWER_EL   	0x24			// Data Abort from a lower Exception level
#define EC_DATA_ABORT_SAME_EL   	0x25			// Data Abort from the same Exception level

#define ABORT_ISS_FSC_TRANS_L0   	0b000100  		// Translation fault from a Data/Instruction Abort exception, level 0
#define ABORT_ISS_FSC_TRANS_L1   	0b000101  		// Translation fault from a Data/Instruction Abort exception, level 1
#define ABORT_ISS_FSC_TRANS_L2   	0b000110  		// Translation fault from a Data/Instruction Abort exception, level 2
#define ABORT_ISS_FSC_TRANS_L3   	0b000111  		// Translation fault from a Data/Instruction Abort exception, level 3

#define ABORT_ISS_WNR_READ			0b0				// Fault caused by an instruction reading
#define ABORT_ISS_WNR_WRITE			0b1				// Fault caused by an instruction writing

#define FSC(iss)					((iss & 0x3f) >> 0)		// Exract FSC (Both Data and Instruction) from ISS
#define WNR(iss)					((iss & 0x40) >> 6)		// Exract WnR from Data Abort ISS

typedef union {
	uint64_t value;
	struct {
		uint64_t iss :  25;		// ISS [5:0]: DFSC/IFSC if it is Data/Instruction Abort
		uint64_t il :   1;
		uint64_t ec :   6;		// Exception Class
		uint64_t iss2 : 24;
		uint64_t res0 : 8;
	};
} esr_el1_t;

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
#ifndef REG_INTERRUPT_H_
#define REG_INTERRUPT_H_

#include "reg_base.h"

#define IRQ_BASIC_PENDING	((volatile unsigned int*)(INTERRUPT_BASE+0x00000200))
#define IRQ_PENDING_1		((volatile unsigned int*)(INTERRUPT_BASE+0x00000204))
#define IRQ_PENDING_2		((volatile unsigned int*)(INTERRUPT_BASE+0x00000208))
#define FIQ_CONTROL		    ((volatile unsigned int*)(INTERRUPT_BASE+0x0000020C))
#define ENABLE_IRQS_1		((volatile unsigned int*)(INTERRUPT_BASE+0x00000210))
#define ENABLE_IRQS_2		((volatile unsigned int*)(INTERRUPT_BASE+0x00000214))
#define ENABLE_BASIC_IRQS	((volatile unsigned int*)(INTERRUPT_BASE+0x00000218))
#define DISABLE_IRQS_1		((volatile unsigned int*)(INTERRUPT_BASE+0x0000021C))
#define DISABLE_IRQS_2		((volatile unsigned int*)(INTERRUPT_BASE+0x00000220))
#define DISABLE_BASIC_IRQS	((volatile unsigned int*)(INTERRUPT_BASE+0x00000224))

#endif // REG_INTERRUPT_H_
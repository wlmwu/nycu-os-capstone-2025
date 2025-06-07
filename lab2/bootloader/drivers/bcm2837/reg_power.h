#ifndef REG_POWER_H_
#define REG_POWER_H_

#include "reg_base.h"

#define POWER_RSTC     ((volatile unsigned int*)(MMIO_BASE+0x10001c))
#define POWER_WDOG     ((volatile unsigned int*)(MMIO_BASE+0x100024))

#endif // REG_MAILBOX_H_
#ifndef REG_GPIO_H_
#define REG_GPIO_H_

#include "reg_base.h"

#define GPFSEL1         ((volatile unsigned int *)(MMIO_BASE+0x00200004))
#define GPPUD           ((volatile unsigned int *)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int *)(MMIO_BASE+0x00200098))

#endif // REG_GPIO_H_
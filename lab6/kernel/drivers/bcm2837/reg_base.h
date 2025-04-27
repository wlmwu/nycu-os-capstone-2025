#ifndef REG_BASE_H_
#define REG_BASE_H_

#define VA_BASE         0xffff000000000000 
#define MMIO_BASE       VA_BASE + 0x3F000000
#define MAILBOX_BASE    MMIO_BASE + 0xb880
#define INTERRUPT_BASE  MMIO_BASE + 0xb000

#endif // REG_BASE_H_
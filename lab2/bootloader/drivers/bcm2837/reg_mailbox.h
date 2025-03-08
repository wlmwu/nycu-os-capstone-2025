#ifndef REG_MAILBOX_H_
#define REG_MAILBOX_H_

#include "reg_base.h"

#define MAILBOX_READ     ((volatile unsigned int*)(MAILBOX_BASE+0x00))
#define MAILBOX_STATUS   ((volatile unsigned int*)(MAILBOX_BASE+0x18))
#define MAILBOX_WRITE    ((volatile unsigned int*)(MAILBOX_BASE+0x20))

#define MAILBOX_POLL     ((volatile unsigned int*)(MAILBOX_BASE+0x10))
#define MAILBOX_SENDER   ((volatile unsigned int*)(MAILBOX_BASE+0x14))
#define MAILBOX_CONFIG   ((volatile unsigned int*)(MAILBOX_BASE+0x1C))

#endif // REG_MAILBOX_H_

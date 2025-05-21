#ifndef DEVICE_H_
#define DEVICE_H_

#include <stdint.h>

typedef uint32_t dev_t; 

#include "vfs.h"
#include "fs.h"

// Device IDs
#define DEV_UART_DEV            0x0100
#define DEV_FRAMEBUFFER_DEV     0x0200

void dev_init();

int dev_register(dev_t dev, struct file_operations *fops);
struct file_operations* dev_get_driver(dev_t dev);

#endif // DEVICE_H_
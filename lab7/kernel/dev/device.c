#include "device.h"
#include "vfs.h"
#include "fs.h"
#include "slab.h"
#include "utils.h"
#include "uart.h"
#include "framebuffer.h"
#include <stdint.h>

#define NUM_DEV_MAX 16

struct dev_driver {
    dev_t dev;
    struct file_operations *f_ops;
};

static struct dev_driver *drivers[NUM_DEV_MAX];

void dev_init() {
    memset(drivers, 0, sizeof(drivers));
    vfs_mkdir(fs_get_root(), "/dev");
    dev_uart_init();
    dev_fbuf_init();
}

int dev_register(dev_t dev, struct file_operations *fops) {
    for (int i = 0; i < NUM_DEV_MAX; ++i) {
        if (!drivers[i]) {
            drivers[i] = kmalloc(sizeof(struct dev_driver));
            drivers[i]->dev = dev;
            drivers[i]->f_ops = fops;
            return 0;
        }
    }
    return -1;
}

struct file_operations* dev_get_driver(dev_t dev) {
    for (int i = 0; i < NUM_DEV_MAX; ++i) {
        if (drivers[i] && drivers[i]->dev == dev) {
            return drivers[i]->f_ops;
        }
    }
    return NULL;
}

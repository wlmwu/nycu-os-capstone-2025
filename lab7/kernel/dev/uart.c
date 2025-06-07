#include "device.h"
#include "uart.h"
#include "mini_uart.h"
#include "slab.h"
#include "vfs.h"
#include "fs.h"

static int dev_uart_open(struct vnode *fvnode, struct file **target) {
    struct file *file = kmalloc(sizeof(struct file));
    file->vnode = fvnode;
    file->f_pos = 0;
    file->f_ops = fvnode->f_ops;
    file->flags = 0;                // Unused
    *target = file;
    return 0;
}

static int dev_uart_close(struct file *file) {
    kfree(file);
    return 0;
}

static int dev_uart_read(struct file *file, void *buf, size_t count) {
    for (int i = 0; i < count; ++i) {
        ((char*)buf)[i] = uart_async_getc();
    }
    return count;
}

static int dev_uart_write(struct file *file, const void *buf, size_t count) {
    for (int i = 0; i < count; ++i) {
        uart_printf("\033[38;5;194m%c\033[0m", ((char*)buf)[i]);
    }
    return count;
}

static struct file_operations dev_uart_fops = {
    .open = dev_uart_open,
    .close = dev_uart_close,
    .read = dev_uart_read,
    .write = dev_uart_write,
};

void dev_uart_init() {
    dev_register(DEV_UART_DEV, &dev_uart_fops);
    vfs_mknod(fs_get_root(), "/dev/uart", DEV_UART_DEV);
}

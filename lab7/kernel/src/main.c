#include "mini_uart.h"
#include "shell.h"
#include "cpio.h"
#include "fdt.h"
#include "irq.h"
#include "timer.h"
#include "mm.h"
#include "sched.h"
#include "kthread.h"
#include "utils.h"
#include "syscall.h"
#include "fs.h"
#include "vfs.h"

static void test_vfs() {
    struct file *f = kmalloc(sizeof(struct file));
    char bufw1[5] = "test";
    char *bufr1 = kmalloc(strlen(bufw1));

    char bufw2[5] = "TEST";
    char *bufr2 = kmalloc(strlen(bufw2));

    char bufw3[5] = "Test";
    char *bufr3 = kmalloc(strlen(bufw3));

    char bufw4[5] = "TesT";
    char *bufr4 = kmalloc(strlen(bufw4));
    
    vfs_mkdir("/files/");
    vfs_mkdir("/files/mnt");

    vfs_mount("/files/mnt", "tmpfs");

    // Write

    vfs_open("/file1", O_CREAT, &f);
    uart_dbg_printf("/file1:\t\t\tRead buf: %s\t, Write buf: %s\n", bufr1, bufw1);
    vfs_write(f, bufw1, strlen(bufw1));
    vfs_close(f);

    vfs_open("/file2", O_CREAT, &f);
    uart_dbg_printf("/file2:\t\t\tRead buf: %s\t, Write buf: %s\n", bufr2, bufw2);
    vfs_write(f, bufw2, strlen(bufw2));
    vfs_close(f);

    vfs_open("/files/file2", O_CREAT, &f);
    vfs_write(f, bufw3, strlen(bufw3));
    uart_dbg_printf("/files/file2:\t\tRead buf: %s\t, Write buf: %s\n", bufr3, bufw3);
    vfs_close(f);

    vfs_open("/files/mnt/file2", O_CREAT, &f);
    vfs_write(f, bufw4, strlen(bufw4));
    uart_dbg_printf("/files/mnt/file2:\tRead buf: %s\t, Write buf: %s\n", bufr4, bufw4);
    vfs_close(f);

    // Read

    vfs_open("/file1", O_CREAT, &f);
    vfs_read(f, bufr1, strlen(bufw1));
    uart_dbg_printf("/file1:\t\t\tRead buf: %s\t, Write buf: %s\n", bufr1, bufw1);
    vfs_close(f);

    vfs_open("/file2", O_CREAT, &f);
    vfs_read(f, bufr2, strlen(bufw2));
    uart_dbg_printf("/file2:\t\t\tRead buf: %s\t, Write buf: %s\n", bufr2, bufw2);
    vfs_close(f);

    vfs_open("/files/file2", O_CREAT, &f);
    vfs_read(f, bufr3, strlen(bufw3));
    uart_dbg_printf("/files/file2:\t\tRead buf: %s\t, Write buf: %s\n", bufr3, bufw3);
    vfs_close(f);

    vfs_open("/files/mnt/file2", O_CREAT, &f);
    vfs_read(f, bufr4, strlen(bufw4));
    uart_dbg_printf("/files/mnt/file2:\tRead buf: %s\t, Write buf: %s\n", bufr4, bufw4);
    vfs_close(f);
}

int main(void* arg) {   /* The value of arg is `x0` which is 0x8200000 in QEMU, so `*arg` is the pointer points to 0x8200000 */
    uart_init();
    timer_init();

    irq_init();
    
    fdt_init(arg);
    cpio_init();
    
    mm_init();

    sched_init();

    fs_init();
    test_vfs();

    shell_init();
    shell_run();        // Always runs in EL1

    return 0;
}
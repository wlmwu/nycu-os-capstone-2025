#include "mini_uart.h"
#include "shell.h"
#include "cpio.h"
#include "fdt.h"

int main(void* arg) {   /* The value of arg is `x0` which is 0x8200000 in QEMU, so `*arg` is the pointer points to 0x8200000 */
    uart_init();
    shell_init();
    fdt_init(arg);
    cpio_init();
    shell_run();
    return 0;
}
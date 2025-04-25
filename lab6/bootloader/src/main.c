#include "mini_uart.h"
#include "shell.h"
#include "fdt.h"

int main(void *arg) {
    uart_init();
    shell_init();
    fdt_init(arg);
    shell_run();
    return 0;
}
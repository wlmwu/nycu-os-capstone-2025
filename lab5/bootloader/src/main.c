#include "mini_uart.h"
#include "shell.h"

void *g_fdt_header;

int main(void *arg) {
    g_fdt_header = arg;
    uart_init();
    shell_init();
    shell_run();
    return 0;
}
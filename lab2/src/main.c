#include "mini_uart.h"
#include "shell.h"

int main() {
    uart_init();
    shell_init();
    shell_run();
    return 0;
}
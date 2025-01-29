#include "shell.h"

void shell_init() {
    uart_puts(kWelcomeMsg);
    uart_puts("\n");
    uart_puts("Type 'help' to show all available commands.\n");
    uart_puts("\n");
}

void shell_run() {
    char buf[NUM_CMD_RECV_MAX];
    while (1) {
        arrset(buf, 0, NUM_CMD_RECV_MAX);
        uart_puts("# ");
        shell_cmd_read(buf);
        shell_cmd_parse(buf);
    }
}

void shell_cmd_read(char* buf) {
    char c;
    int idx = -1;
    while (idx++ < NUM_CMD_RECV_MAX) {
        c = uart_recv();
        uart_puts(&c);       // Use puts to handle if c == '\n'
        buf[idx] = c;
        if (c == '\n') {
            buf[idx] = '\0';
            break;
        }
    }
}

void shell_cmd_parse(char* buf) {
    for (int i = 0; i < NUM_CMD; ++i) {
        if (strcmp(buf, kCmds[i].command) == 0) {
            kCmds[i].func();
            return;
        }
    }
    if (buf[0] != '\0') {
        uart_puts("Command not found: ");
        uart_puts(buf);
        uart_puts("\n");
    }
}

void command_hello() {
    uart_puts("Hello World!\n");
}

void command_help() {
    for (int i = 1; i < NUM_CMD; ++i) {
        uart_puts(kCmds[i].command);
        uart_puts("\t: ");
        uart_puts(kCmds[i].help);
        uart_puts("\n");
    }
}

void command_info() {
    char buf[NUM_CMD_SEND_MAX];

    mbox_get_info(buf, kTagGetBoardRevision, 1);
    uart_puts("Board Revision:\t\t\t");
    uart_puts(buf);
    uart_puts("\n");

    arrset(buf, 0, NUM_CMD_SEND_MAX);

    mbox_get_info(buf, kTagGetArmMemory, 2);
    uart_puts("ARM Memory Base Address:\t");
    uart_puts(buf);
    uart_puts("\n");
    uart_puts("ARM Memory Size:\t\t");
    uart_puts(&buf[LEN_U32_HEX_STR]);
    uart_puts("\n");
}

void command_reboot() {
    reset(NUM_TICKS);
}

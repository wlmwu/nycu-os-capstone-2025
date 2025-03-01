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
    char c[] = "\0\0";
    int idx = 0;
    while (idx < NUM_CMD_RECV_MAX) {
        c[0] = uart_recv();
        
        if (c[0] == '\n') {                     // Enter
            break;
        } else if (c[0] == 8 || c[0] == 127) {  // Backspace
            if (idx > 0) {
                uart_puts("\033[1D \033[1D");       // '\033[1D' moves cursor back, ' ' erases the character, '\033[1D' moves back again
                idx--;
            }
        } else if (c[0] == 21) {                // Ctrl-U
            uart_puts("\033[2K");                   // Clear line output
            uart_puts("\r# ");                      // Reset the cursor
            idx = 0;
        } else {
            buf[idx] = c[0];
            idx++;
        }

        uart_puts(c);
    }
    uart_puts("\n");
    buf[idx] = '\0'; 
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

void command_help() {
    for (int i = 1; i < NUM_CMD; ++i) {
        uart_puts(kCmds[i].command);
        uart_puts("\t: ");
        uart_puts(kCmds[i].help);
        uart_puts("\n");
    }
}

void command_reboot() {
    reset(NUM_TICKS);
}

void command_bootloader() {
    bootloader_run();
}
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
    int idx = -1;
    while (idx++ < NUM_CMD_RECV_MAX) {
        c[0] = uart_recv();
        uart_puts(c);
        buf[idx] = c[0];
        if (c[0] == '\n') {
            buf[idx] = '\0';
            break;
        }
    }
}

void shell_cmd_parse(char* buf) {
    int MAX_ARGS = 100;
    char* argv[MAX_ARGS]; // Argument array
    int argc = split_args(buf, argv, MAX_ARGS);
    if (argc == 0) {
        return;
    }

    for (int i = 0; i < NUM_CMD; ++i) {
        if (strcmp(buf, kCmds[i].command) == 0) {
            kCmds[i].func(argc, argv);
            return;
        }
    }
    if (buf[0] != '\0') {
        uart_puts("Command not found: ");
        uart_puts(buf);
        uart_puts("\n");
    }
}

void command_hello(int argc, char **argv) {
    uart_puts("Hello World!\n");
}

void command_help(int argc, char **argv) {
    for (int i = 1; i < NUM_CMD; ++i) {
        uart_puts(kCmds[i].command);
        uart_puts("\t: ");
        uart_puts(kCmds[i].help);
        uart_puts("\n");
    }
}

void command_info(int argc, char **argv) {
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

void command_reboot(int argc, char **argv) {
    reset(NUM_TICKS);
}

void command_ls(int argc, char **argv) {
    char *pathname;
    cpio_newc_header_t *hptr = cpio_get_start_file();
    while (hptr) {
        hptr = cpio_get_file(hptr, &pathname, NULL);
        if (!pathname) {
            break;
        }
        uart_puts(pathname);
        uart_puts("\n");
    }
}

void command_cat(int argc, char **argv) {
    if (argc <= 1) {
        uart_puts(" : Is a directory\n");
        return;
    }
    char *target_filename = argv[1];
    cpio_newc_header_t *hptr = cpio_get_file_by_name(target_filename);
    if (!hptr) {
        uart_puts(target_filename);
        uart_puts(": No such file or directory\n");
        return;
    }

    char *pathname;
    char *filedata;
    cpio_get_file(hptr, &pathname, &filedata);
    if (filedata) {
        uart_puts(filedata);
        uart_puts("\n");
    }
}
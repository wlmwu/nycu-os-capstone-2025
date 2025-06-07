#include "shell.h"
#include "mini_uart.h"
#include "mailbox.h"
#include "utils.h"
#include "cpio.h"
#include "timer.h"
#include "kthread.h"
#include "irq.h"
#include "slab.h"
#include "proc.h"
#include "power.h"

void shell_init() {
    uart_puts(kWelcomeMsg);
    uart_puts("\n");
    uart_puts("Type 'help' to show all available commands.\n");
    uart_puts("\n");
}

void shell_run() {
    char buf[NUM_CMD_RECV_MAX];
    while (1) {
        memset(buf, 0, sizeof(buf));
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
    uint32_t buf[2];    // kTagGetArmMemory returns two values
    
    mbox_get_info(buf, kTagGetBoardRevision, 1);
    uart_printf("Board Revision:\t\t\t%x\n", buf[0]);
    mbox_get_info(buf, kTagGetArmMemory, 2);
    uart_printf("ARM Memory Base Address:\t%x\n", buf[0]);
    uart_printf("ARM Memory Size:\t\t%x\n", buf[1]);
}

void command_reboot(int argc, char **argv) {
    const int kNumTicks = 100;
    power_reset(kNumTicks);
}

void command_ls(int argc, char **argv) {
    char *pathname;
    cpio_newc_header_t *hptr = cpio_get_start_file();
    while (hptr) {
        hptr = cpio_get_file(hptr, &pathname, NULL, NULL);
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
    cpio_get_file(hptr, &pathname, NULL, &filedata);
    if (filedata) {
        uart_puts(filedata);
        uart_puts("\n");
    }
}

void command_exec(int argc, char **argv) {
    if (argc < 2) {
        uart_puts("Usage: exec <program.img>\n");
        return;
    }
    irq_disable();
    char *filename = argv[1];
    void *prog = NULL;
    size_t progsize = 0;
    proc_load_prog(filename, &prog, &progsize);
    if (!prog) {
        irq_enable();
        return;
    }
    proc_create(prog, NULL, progsize);

    sched_start();     // Jump to thread queue
    irq_enable();
}

static void echoat_timer_event(void *msg) {
    uart_printf("\nechoat: %u %s\n", timer_tick_to_second(timer_get_current_tick()) , (char*) msg);
}

void command_echoat(int argc, char** argv) {
    if (argc < 3) {
        uart_printf("Usage: echoat <message> <seconds>\n");
        return;
    }
    char *message = argv[1];
    uint64_t seconds = stoi(argv[2], NULL, 10);
    
    timer_add_event(echoat_timer_event, (void *)message, strlen(message) + 1, timer_second_to_tick(seconds));
}

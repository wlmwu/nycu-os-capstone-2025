#ifndef SHELL_H_
#define SHELL_H_

#include "mini_uart.h"
#include "utils.h"
#include "bootloader.h"

#define NUM_CMD 3
#define LEN_CMD_NAME_MAX 16
#define LEN_CMD_HELP_MAX 128

#define NUM_CMD_SEND_MAX 128
#define NUM_CMD_RECV_MAX 128

typedef void (*command_fn_t)(void);

typedef struct shell_cmd {
    char command[LEN_CMD_NAME_MAX];
    char help[LEN_CMD_HELP_MAX];
    command_fn_t func;
} shell_cmd_t;

void command_help(void);
void command_reboot(void);
void command_bootloader(void);

static const shell_cmd_t kCmds[NUM_CMD] = {
    {.command = "help", .help = "Print all available commands", .func = command_help},
    {.command = "load", .help = "Get device information", .func = command_bootloader},
    {.command = "reboot", .help = "Reboot the device", .func = command_reboot},
};

void shell_init();
void shell_run();

void shell_cmd_read(char*);
void shell_cmd_parse(char*);

static const char* kWelcomeMsg = "\n"
    "\033[1;95m    ____      ___      ___                  \033[1;96m   ____      ____      ____      ____  \n"
    "\033[1;95m   F __ ]    F __\".  ,\"___\".                \033[1;96m  / _  `.   F _  ]    / _  `.   F ___L \n"
    "\033[1;95m  J |--| L  J (___|  FJ---L]                \033[1;96m J_/-7 .'  J |/ | L  J_/-7 .'  J |___| \n"
    "\033[1;95m  | |  | |  J\\___ \\ J |   LJ \033[1;93m               \033[1;96m `-:'.'.'  | | /| |  `-:'.'.'  |____ \\  \n"
    "\033[1;95m  F L__J J .--___) \\| \\___--.\033[0;33m       \033[1;33mN Y C U \033[1;96m .' ;_J__  F  /_J J  .' ;_J__ .--___) \\ \n"
    "\033[1;95m J\\______/FJ\\______J \\_____/F\033[0;33m     o\033[0;33m O       \033[1;96mJ________LJ\\______/FJ________LJ\\______J \n"
    "\033[1;95m  J______F  J______F J_____F \033[0;33m    o  \033[1;92m  ____  \033[1;96m|________| J______F |________| J______F \033[1;92m\n"
    "                                ][_,__|[]|\n"
    "__|\"\"\"\"\"|___|\"\"\"\"\"|___|\"\"\"\"\"|  {|_|_|_|__|__|\"\"\"\"\"|___|\"\"\"\"\"|___|\"\"\"\"\"|___|\"\"\"\"\"| \n"
    "\"\"`-0-0-'\"\"\"`-0-0-'\"\"\"`-0-0-'.//oo-oo-OOO'\"\"`-0-0-'\"\"\"`-0-0-'\"\"\"`-0-0-'\"\"\"`-0-0-'\033[0m   \n";

#endif // SHELL_H_
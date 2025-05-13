#ifndef SHELL_H_
#define SHELL_H_

#define NUM_CMD 8
#define LEN_CMD_NAME_MAX 16
#define LEN_CMD_HELP_MAX 128

#define NUM_CMD_SEND_MAX 128
#define NUM_CMD_RECV_MAX 128

#define USER_STACK_SIZE  (4 * 1024)  // 4KB user stack

typedef void (*command_fn_t)(int argc, char **argv);

typedef struct shell_cmd {
    char command[LEN_CMD_NAME_MAX];
    char help[LEN_CMD_HELP_MAX];
    command_fn_t func;
} shell_cmd_t;

void command_help(int, char**);
void command_hello(int, char**);
void command_info(int, char**);
void command_reboot(int, char**);
void command_ls(int, char**);
void command_cat(int, char**);
void command_exec(int, char**);
void command_echoat(int, char**);

static const shell_cmd_t kCmds[NUM_CMD] = {
    {.command = "help", .help = "Print all available commands", .func = command_help},
    {.command = "hello", .help = "Print \"Hello World!\"", .func = command_hello},
    {.command = "info", .help = "Get device information", .func = command_info},
    {.command = "reboot", .help = "Reboot the device", .func = command_reboot},
    {.command = "ls", .help = "List files in the root directory", .func = command_ls},
    {.command = "cat", .help = "Display file contents", .func = command_cat},
    {.command = "exec", .help = "Execute a user program", .func = command_exec},
    {.command = "echoat", .help = "Print a message after a delay", .func = command_echoat},
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
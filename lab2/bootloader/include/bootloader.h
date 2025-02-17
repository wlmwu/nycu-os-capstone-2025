#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#include "mini_uart.h"
#include "fdt.h"
#include "utils.h"

#define BOOTLOADER_LOAD_ADDR 0x100000
#define KERNEL_LOAD_ADDR 0x80000

void bootloader_run();
void bootloader_relocate();
void bootloader_load();

#endif // BOOTLOADER_H_

#ifndef BOOTLOADER_H_
#define BOOTLOADER_H_

#define BOOTLOADER_LOAD_ADDR 0x60000
#define KERNEL_LOAD_ADDR 0x80000

#ifndef __ASSEMBLER__

void bootloader_load();

#endif //__ASSEMBLER__

#endif // BOOTLOADER_H_

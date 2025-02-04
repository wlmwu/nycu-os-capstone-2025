CROSS_COMPILE_TOOL = aarch64-none-elf
CC = $(CROSS_COMPILE_TOOL)-gcc
LD = $(CROSS_COMPILE_TOOL)-ld
OBJCOPY = $(CROSS_COMPILE_TOOL)-objcopy
CFLAGS = -mcpu=cortex-a53+nosimd -nostdlib -ffreestanding
LDFLAGS = -m aarch64elf -nostdlib -T linker.ld
OUTPUT = kernel8.img
ELF = kernel8.elf
SRC_DIR = src
FS_DIR = fs
SHELL_DIR = shell
DRIVER_DIR = drivers
BOOT_DIR = boot
INC_DIR = include
BUILD_DIR = build
CPIO_ARCHIVE = ../initramfs.cpio

C_SRC_FILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(INC_DIR)/*.c) $(wildcard $(DRIVER_DIR)/*.c) $(wildcard $(SHELL_DIR)/*.c) $(wildcard $(FS_DIR)/*.c)
ASM_SRC_FILES = $(wildcard $(SRC_DIR)/*.S) $(wildcard $(BOOT_DIR)/*.S)

BUILD_DIR_C = $(C_SRC_FILES:%.c=$(BUILD_DIR)/%.o)
BUILD_DIR_ASM = $(ASM_SRC_FILES:%.S=$(BUILD_DIR)/%.o)
OBJ_FILES = $(BUILD_DIR_C) $(BUILD_DIR_ASM)

all: clean $(OUTPUT)

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -I$(INC_DIR) -I$(SHELL_DIR) -I$(DRIVER_DIR) -I$(FS_DIR) -c $< -o $@

$(ELF): $(OBJ_FILES)
	$(LD) $(LDFLAGS) -o $@ $^

$(OUTPUT): $(ELF)
	$(OBJCOPY) --output-target=aarch64-rpi3-elf -O binary $< $@

clean:
	rm -rf $(BUILD_DIR) $(ELF) $(OUTPUT)

qemu:
	qemu-system-aarch64 -M raspi3b -kernel $(OUTPUT) -serial null -serial stdio -display none -initrd $(CPIO_ARCHIVE)

qemu-pty:
	qemu-system-aarch64 -M raspi3b -kernel $(OUTPUT) -serial null -serial pty -display none

gdb:
	qemu-system-aarch64 -M raspi3b -kernel $(OUTPUT) -serial null -serial pty -display none -S -s

.PHONY: all clean
CROSS_COMPILE_TOOL = aarch64-none-elf
CC = $(CROSS_COMPILE_TOOL)-gcc
LD = $(CROSS_COMPILE_TOOL)-ld
OBJCOPY = $(CROSS_COMPILE_TOOL)-objcopy
CFLAGS = -mcpu=cortex-a53+nosimd -nostdlib -ffreestanding
LDFLAGS = -m aarch64elf -nostdlib -T linker.ld
OUTPUT = userprog.img
ELF = userprog.elf

BUILD_DIR = build
ROOTFS_DIR = ../../rootfs
CPIO_ARCHIVE = ../initramfs.cpio
DT_ARCHIVE = ../bcm2710-rpi-3-b-plus.dtb

INC_DIRS := $(shell find . -type f -name "*.h" -exec dirname {} \; | sort -u)
INCLUDE_FLAGS := $(addprefix -I, $(INC_DIRS))
C_SRC_FILES := $(shell find . -type f -name "*.c")
ASM_SRC_FILES := $(shell find . -type f -name "*.S")

BUILD_DIR_C = $(C_SRC_FILES:%.c=$(BUILD_DIR)/%.o)
BUILD_DIR_ASM = $(ASM_SRC_FILES:%.S=$(BUILD_DIR)/%.o)
OBJ_FILES = $(BUILD_DIR_C) $(BUILD_DIR_ASM)

all: clean $(OUTPUT) $(CPIO_ARCHIVE)

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE_FLAGS) -c $< -o $@

$(ELF): $(OBJ_FILES)
	$(LD) $(LDFLAGS) -o $@ $^

$(OUTPUT): $(ELF)
	$(OBJCOPY) --output-target=aarch64-rpi3-elf -O binary $< $@

$(CPIO_ARCHIVE): $(ROOTFS_DIR) $(OUTPUT)
	@mkdir -p $(ROOTFS_DIR)
	@cp $(OUTPUT) $(ROOTFS_DIR)
	cd $(ROOTFS_DIR); find . | cpio -o -H newc > $(CPIO_ARCHIVE)

clean:
	rm -rf $(BUILD_DIR) $(ELF) $(OUTPUT)

QEMU = qemu-system-aarch64
QEMUFLAGS = -M raspi3b -kernel $(OUTPUT) -serial null -display none -initrd $(CPIO_ARCHIVE) -dtb $(DT_ARCHIVE)

QEMUFLAGS += $(if $(findstring PTY,$(MAKECMDGOALS)), -serial pty, -serial stdio)
QEMUFLAGS += $(if $(findstring GDB,$(MAKECMDGOALS)), -S -s,)

run:
	$(QEMU) $(QEMUFLAGS)


.PHONY: all clean
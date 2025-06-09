# Lab 4: Allocator

Here implements a bootloader and a kernel. The bootloader is a kernel that loads the real kernel image, and the kernel performs essential system operations to get the device running. Additionally, a Python tool is provided for transferring the kernel image to the bootloader via UART.

## Project Structure
### Bootloader
Contains the bootloader code. It loads the actual kernel image from the SD card or UART.

### Kernel
Contains the kernel code. It can be loaded by the bootloader or run independently. 

### Host Tool (`sendimg.py`)
This Python script is used to send the kernel image over UART from the host machine to the bootloader kernel.  
```bash
python host_tools/sendimg.py <KERNEL_PATH> -p <SERIAL_PORT> -b <BAUDRATE>
```

### Root File System
A sample root file system, containing files that the kernel can access once it is booted.  
- Example files: `file1.txt`, `fileTwoPlusALongName.txt`

### User Program
This folder contains a program designed to test exception handling within the kernel.

---

## Build and Run
This section details the step-by-step process to build the kernels and run them on the Raspberry Pi 3B+. Both the `bootloader/` and `kernel/` directories can be compiled into `kernel8.img` files.

### 1. Building the Bootloader
First, compile the bootloader kernel. This `kernel8.img` will be placed on the SD card.
```bash
cd bootloader
make
```

### 2. Building the Kernel
Next, compile the main kernel. This `kernel8.img` will be transferred to the Raspberry Pi via UART by the bootloader.
```bash
cd kernel
make
```

### 3. Preparing the SD Card
To prepare your SD card for booting the bootloader, you'll need a few essential files.
1. Burn `nycuos.img` (provided in Lab0) to your SD card: 
    - `bootcode.bin` 
    - `fixup.dat` 
    - `start.elf`
2. Copy additional files from this repository to the SD card:
    - `initramfs.cpio`
    - `bcm2710-rpi-3-b-plus.dtb`
    - `config.txt`
3. Copy the compiled `bootloader/kernel8.img` to the SD card.

Now the SD card should have the following structure:
```bash
.
├── bootcode.bin
├── fixup.dat
├── start.elf
├── kernel8.img (bootloader kernel)
├── initramfs.cpio
├── bcm2710-rpi-3-b-plus.dtb
└── config.txt
```

### 4. Running on Raspberry Pi 3B+ (UART Transfer)
Once your SD card is set up with the bootloader, you can transfer and run the main kernel over UART.
1. Connect to the Raspberry Pi using a serial terminal on your host machine (e.g., using picocom or minicom).
```bash
picocom -b 115200 /dev/path-to-usbserial
```
2. Once the bootloader starts, type load to begin the kernel loading process.
3. Open a second terminal on your host machine and use the `sendimg.py` to transfer the `kernel8.img`.
```bash
python host_tools/sendimg.py kernel/kernel8.img -p /dev/path-to-usbserial -b 115200
```
Upon successful transfer, the main kernel will begin execution on the Raspberry Pi.
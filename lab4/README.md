# Lab 4: Allocator

Here implements a bootloader and a kernel. The bootloader is a kernel that loads the real kernel image, and the kernel performs essential system operations to get the device running. Additionally, a Python tool is provided for transferring the kernel image to the bootloader via UART.

## Bootloader
The bootloader initializes the hardware and loads the real kernel image. If the bootloader kernel is installed on the SD card, users can load the kernel without manually copying the image to the SD card.
- Location: `bootloader/`

## Kernel
The kernel performs essential operations to get the system running. It can be loaded by the bootloader or run independently. 
- Location: `kernel/`

## Host Tool (`sendimg.py`)
This Python script is used to send the kernel image over UART from the host machine to the bootloader kernel.  
```bash
python host_tools/sendimg.py <KERNEL_PATH> -p <SERIAL_PORT> -b <BAUDRATE>
```

## Root File System
A sample root file system, containing files that the kernel can access once it is booted.  
- Location: `rootfs/`  
- Example files: `file1.txt`, `fileTwoPlusALongName.txt`

## User Program
This folder contains a program designed to test exception handling within the kernel.
- Location: `user/`
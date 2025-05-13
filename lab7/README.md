# Lab 6: Virtual Memory

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
This directory contains the initial set of files that are packaged into an `initramfs.cpio`. The kernel will access files in `initramfs.cpio` upon boot.
- Location: `rootfs/`  
- Example files: `file1.txt`, `fileTwoPlusALongName.txt`

While the repository might show a limited number of files, the actual `initramfs.cpio` includes:
- `file1.txt`
- `fileTwoPlusALongName.txt`
- `userprog.img`: A user program created by `users/user1`.
- `forktest.img`:  A user program created by `users/user2`.
- `syscall.img`: A user program provided by the lab 5.
- `vm.img`: A user program provided by the lab 6.

## User Program
This directory now contains multiple user programs designed to test different kernel functionalities:
- Location: `users/`
    - `user1`: This program creates `userprog.img` to test the kernel's exception handling mechanisms.
    - `user2`: This program creates `forktest.img` to test the implementation of the system calls.
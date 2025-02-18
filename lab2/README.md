# Lab2: Booting

Here implements a bootloader and a kernel. The bootloader is a kernel that loads the real kernel image, and the kernel performs basic operations to get the system running. Additionally, a Python tool is provided for transferring the kernel image to the bootloader via UART.

## Bootloader
The bootloader initializes the hardware and loads the real kernel image.  
- Location: `bootloader/`

## Kernel
The kernel, built to be loaded by the bootloader, performs basic operations to get the system running.  
- Location: `kernel/`

## Host Tool (`sendimg.py`)
This Python script is used to send the kernel image over UART from the host machine to the bootloader kernel.  
```bash
python host_tools/sendimg.py <KERNEL_PATH> -p <SERIAL_PORT> -b <BAUDRATE>
```

## Root File System (rootfs)
A sample root file system, containing files that the kernel can access once it is booted.  
- Location: `rootfs/`  
- Example files: `file1.txt`, `fileTwoPlusALongName.txt`
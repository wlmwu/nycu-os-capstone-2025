# Raspberry-Pi Bare Metal Labs

This repository contains the implementation of the **NYCU Operating System Capstone** labs. The full lab specifications are available at: https://nycu-caslab.github.io/OSC2025/

## Overview

Each lab builds upon the previous lab to gradually implement core functions:

* **Lab 0: Environment Setup** - Setting up the development environment.
* **Lab 1: Hello World** - Practicing bare-metal programming by implementing a simple shell and enabling UART communication.
* **Lab 2: Booting** - Implementing a bootloader to load the kernel via UART, and understanding initial ramdisk and devicetree concepts.
* **Lab 3: Exception and Interrupt** - Understanding and handling exceptions, interrupts, and how the Raspberry Pi 3's peripherals interact with the CPU via interrupt controllers.
* **Lab 4: Allocator** - Implementing essential memory allocators to manage physical memory.
* **Lab 5: Thread and User Process** - Understanding thread management, context switching, and POSIX signals.
* **Lab 6: Virtual Memory** - Enabling the MMU to establish isolated virtual address spaces for the kernel and user processes, and implementing demand paging and copy-on-write.
* **Lab 7: Virtual File System** - Implementing a Virtual File System interface, set up a root file system and create special files for UART and framebuffer.

Each lab directory includes a dedicated `README.md` that documents how to build and test that specific lab.

## Prerequisites

- **Target Board**: Raspberry Pi 3B+
- **Host OS**: macOS
- **Toolchain**: `aarch64-none-elf`

## Additional Notes and Resources

During the implementation of these labs, I maintained a detailed set of notes, which can be found at [Operating System Capstone Notes](https://hackmd.io/@wlmwu/rJjcciMmge). These notes cover:

* Toolchain and environment setup
* Step-by-step implementation logic
* Essential concepts and background knowledge required for the labs.
* Debugging techniques and encountered issues
* References to relevant open-source implementations

These notes aim to provide a deeper understanding and serve as a guide to my bare-metal kernel development journey.
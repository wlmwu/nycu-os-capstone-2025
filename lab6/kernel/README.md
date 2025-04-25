# Kernel

## Build Instructions

To build the kernel, simply run:

```bash
make
```

This will compile the kernel and produce the final bootloader image (`kernel8.img`).

## Running the  Kernel

## Run with QEMU

To run the bootloader kernel using QEMU, simply run:

```bash
make run
```

### Run with QEMU and GDB

1. For debugging, you can run the kernel in QEMU and connect to it using GDB. Use the following command to start the simulation and set up GDB:

   ```bash
   make run GDB
   ```

2. In the second window, start GDB to debug the kernel:

   ```bash
   aarch64-elf-gdb
   ```
   You can start debugging immediately.

Alternatively, you can use the `debug.sh` script to automate the process:

```bash
./debug.sh
```
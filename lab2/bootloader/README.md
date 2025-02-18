# Bootloader Kernel

## Build Instructions

To build the bootloader kernel, simply run:

```bash
make
```

This will compile the kernel and produce the final bootloader image (`kernel8.img`).

## Running the Bootloader Kernel

### Run with QEMU

To run the bootloader kernel using QEMU, simply run:

```bash
make run
```

### Run with QEMU and Serial PTY

To run the bootloader kernel with QEMU and use a serial PTY, follow these steps:

1. Start QEMU with the `PTY` flag:

   ```bash
   make run PTY
   ```

   This will start QEMU with serial PTY.

2. In a separate terminal window, use `picocom` to connect to the serial output. You'll need to specify the serial port number that QEMU printed in the previous step:

   ```bash
   picocom -b 115200 /dev/ttys<port>
   ```

   Replace `<port>` with the actual serial port number, which will be printed in the output of the first terminal.

Alternatively, you can use the `debug.sh` script to automate the process:

```bash
./debug.sh
```

**Note**: You need to install `tmux` before using the `debug.sh` script.

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

### Run with QEMU - Serial PTY and GDB
To run with both QEMU and GDB with serial PTY, follow these steps:

1. Start QEMU with both the `PTY` and `GDB` flags:

   ```bash
   make run GDB PTY
   ```

2. In the second window, use `picocom` to connect to the serial output. The serial port number will be displayed in the output of the first window when QEMU starts:

   ```bash
   picocom -b 115200 /dev/ttys<port>
   ```

   Replace `<port>` with the actual serial port number, which will be printed in the output of the first terminal.

3. In the third window, start GDB to debug the kernel:

   ```bash
   aarch64-elf-gdb
   ```
   You can start debugging immediately.
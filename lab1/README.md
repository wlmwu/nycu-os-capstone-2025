# Lab1: Hello World

## Running with QEMU

To run the program using QEMU, follow these steps:

1. Build the program:

   ```bash
   make
   ```

2. Run the program with QEMU:

   ```bash
   make qemu
   ```

## Running on Raspberry Pi 3B+

To run the program on a Raspberry Pi, follow these steps:

1. Build the program:

   ```bash
   make
   ```

2. Prepare the SD card:
   - Copy the `kernel8.img` file to the SD card.
   - The SD card should already contain the following files (you can retrieve them from `nycuos.img` in lab0):
     - `bootcode.bin`
     - `start.elf`
     - `fixup.dat`

3. Insert the SD card into your Raspberry Pi.

4. Open a terminal and connect to the Raspberry Pi via serial:

   ```bash
   screen /dev/tty.usbserial-0001 115200
   ```

   *(Make sure to change the serial device to match your system's configuration.)*
# Building the Kernel
To compile the kernel:
```sh
make
```
This will generate `kernel8.img` in the project directory.

# Running the Kernel in QEMU
To launch QEMU:
```sh
make qemu-pty
```
Then, in a separate terminal, connect to the serial output:
```sh
picocom -b 115200 /dev/ttys031
```
*(Ensure you replace `/dev/ttys031` with the correct device name displayed by QEMU.)*

# Debugging with GDB
1. Start QEMU in debugging mode:
   ```sh
   make gdb
   ```
2. Connect with picocom:
   ```sh
   picocom -b 115200 /dev/ttys031
   ```
3. Launch GDB and connect to the remote target:
   ```sh
   aarch64-elf-gdb
   (gdb) file kernel8.elf
   (gdb) target remote :1234
   ```
Now you can set breakpoints and debug the kernel.
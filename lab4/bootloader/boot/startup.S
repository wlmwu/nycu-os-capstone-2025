#include "bootloader.h"

.section ".text.boot"
.globl _start

_start:
    mrs    x2, mpidr_el1        // Move Register from system. Get the processor ID from mpidr_el1 register
    and    x2, x2, #0xFF        // Isolate the CPU core ID (lowest 8 bits)
    cbz    x2, relo           // If core ID is 0 (primary core), jump to 'master' to initialize

hang:
    b      hang                 // Secondary cores hang here in an infinite loop

relo:
    ldr    x1, =_start
    ldr    x2, =BOOTLOADER_LOAD_ADDR

    // If already running at the target address, skip relocation
    cmp    x1, x2
    b.eq   entry

    ldr    x3, =_end
    sub    x4, x3, x1           // x4 = Size to copy

.Lloop:
    ldr    x5, [x1], #8         // Load 8 bytes from source, post-increment source
    str    x5, [x2], #8         // Store 8 bytes to destination, post-increment destination
    subs   x4, x4, #8           // Decrement remaining size by 8 bytes and update condition flag
    b.ne   .Lloop               // Loop if more bytes to copy

    ldr    x1, =_start
    ldr    x2, =BOOTLOADER_LOAD_ADDR
    sub    x3, x2, x1           // x3 = Offset between original and relocated
    ldr    x1, =master
    add    x1, x1, x3
    br     x1                   // Branch to the relocated master (absolute jump)

master:
    adr    x2, __bss_start      // Load the address of the start of the .bss section
    adr    x1, __bss_end        // Load the address of the end of the .bss section
    sub    x1, x1, x2           // Calculate the size of the .bss section
    bl     memzero              // Call memzero to zero out the .bss section

    adr    x2, _stack_end       // Copy the end of the stack into a general purpose register first
    mov    sp, x2               // Load the stack pointer with the register

entry:
    bl     main                 // Jump to the "main" (defined in .c)
    b      hang                    

memzero:
    cbnz   x1, zero_loop        // If x1 != 0, jump to zero_loop
    ret                         // Return if no more memory to zero

zero_loop:
    str    xzr, [x2], #8        // Store zero at address in x2, increment x2 by 8 bytes
    subs   x1, x1, #8           // Subtract 8 from x1
    cbnz   x1, zero_loop        // Repeat the loop if x1 != 0
    ret                         // Return when done
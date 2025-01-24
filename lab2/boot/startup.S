.section ".text.boot"
.globl _start

_start:
    mrs    x0, mpidr_el1        // Move Register from system. Get the processor ID from mpidr_el1 register
    and    x0, x0, #0xFF        // Isolate the CPU core ID (lowest 8 bits)
    cbz    x0, master           // If core ID is 0 (primary core), jump to 'master' to initialize

hang:
    b      hang                 // Secondary cores hang here in an infinite loop

master:
    ldr    x0, =__bss_start     // Load the address of the start of the .bss section
    ldr    x1, =__bss_end       // Load the address of the end of the .bss section
    sub    x1, x1, x0           // Calculate the size of the .bss section
    bl     memzero              // Call memzero to zero out the .bss section

    ldr    x0, =_stack_end      // Copy the end of the stack into a general purpose register first
    mov    sp, x0               // Load the stack pointer with the register
    bl     main                 // Jump to the "main" (defined in .c)
    b      hang                    

memzero:
    cbnz   x1, zero_loop        // If x1 != 0, jump to zero_loop
    ret                         // Return if no more memory to zero

zero_loop:
    str    xzr, [x0], #8        // Store zero at address in x0, increment x0 by 8 bytes
    subs   x1, x1, #8           // Subtract 8 from x1
    cbnz   x1, zero_loop        // Repeat the loop if x1 != 0
    ret                         // Return when done




.section ".text.boot"
.globl _start

_start:
    mrs    x2, mpidr_el1        // Move Register from system. Get the processor ID from mpidr_el1 register
    and    x2, x2, #0xFF        // Isolate the CPU core ID (lowest 8 bits)
    cbz    x2, master           // If core ID is 0 (primary core), jump to 'master' to initialize

hang:
    b      hang                 // Secondary cores hang here in an infinite loop

master:
    bl     from_el2_to_el1
    bl     set_exception_vector_table
    bl     core_timer_enable

    ldr    x2, =__bss_start     // Load the address of the start of the .bss section
    ldr    x1, =__bss_end       // Load the address of the end of the .bss section
    sub    x1, x1, x2           // Calculate the size of the .bss section
    bl     memzero              // Call memzero to zero out the .bss section

    ldr    x2, =_stack_end      // Copy the end of the stack into a general purpose register first
    mov    sp, x2               // Load the stack pointer with the register
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

from_el2_to_el1:
    mov    x1, (1 << 31)        
    msr    hcr_el2, x1          // Setting hcr_el2 bit[31] to 1 means EL1 uses aarch64
    mov    x1, 0x3c5            // EL1h (SPSel = 1) with interrupt disabled
    msr    spsr_el2, x1
    msr    elr_el2, lr
    eret                        // return to EL1

set_exception_vector_table:
    adr    x1, exception_vector_table
    msr    vbar_el1, x1
    ret

core_timer_enable:
    mov    x2, 1
    msr    cntp_ctl_el0, x2     // Timer enabled
    ret

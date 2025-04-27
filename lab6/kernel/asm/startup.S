#include "mmu.h"

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
    bl     enable_mmu
    mrs    x3, sctlr_el1
    orr    x3 , x3, 1
    msr    sctlr_el1, x3                // Enable MMU
    
    ldr    x1, =_va_trampoline
    br     x1
_va_trampoline: 
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

enable_mmu:
    // Configure TCR
    ldr    x1, =TCR_CONFIG_DEFAULT
    msr    tcr_el1, x1
    
    // Configure MAIR
    ldr    x1, =( (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE_nGnRnE * 8)) | (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL_NOCACHE * 8)) )
    msr    mair_el1, x1

    // Setup page tables
    ldr    x1, =PGTABLE_START_ADDR      // x1: PGD address
    add    x2, x1, #PAGE_SIZE           // x2: PUD address

    // PGD entry (point to PUD)
    ldr    x3, =BOOT_PGD_ATTR
    orr    x3, x2, x3                   // Combine the physical address of next level page with attribute
    str    x3, [x1]                     // Store to the first entry of PGD

    // PUD entries (point to 1GB block)
    ldr    x3, =BOOT_PUD_ATTR
    mov    x4, 0x00000000               // First entry (1GB block)
    orr    x4, x3, x4
    str    x4, [x2]
    mov    x4, 0x40000000               // Second entry (1GB block)
    orr    x4, x3, x4
    str    x4, [x2, 8]

    msr    ttbr0_el1, x1                // Load PGD
    msr    ttbr1_el1, x1
   
    ret    
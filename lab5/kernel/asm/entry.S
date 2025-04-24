    // save general registers to stack
    // The layout here should be aligned with trapframe_t defined in exception.h
    .macro save_all
        sub sp, sp, 34 * 8                // Allocate space for 34 registers (x0-x30 + elr_el1 + spsr_el1 + sp+el0)
        stp x0, x1, [sp ,16 * 0]
        stp x2, x3, [sp ,16 * 1]
        stp x4, x5, [sp ,16 * 2]
        stp x6, x7, [sp ,16 * 3]
        stp x8, x9, [sp ,16 * 4]
        stp x10, x11, [sp ,16 * 5]
        stp x12, x13, [sp ,16 * 6]
        stp x14, x15, [sp ,16 * 7]
        stp x16, x17, [sp ,16 * 8]
        stp x18, x19, [sp ,16 * 9]
        stp x20, x21, [sp ,16 * 10]
        stp x22, x23, [sp ,16 * 11]
        stp x24, x25, [sp ,16 * 12]
        stp x26, x27, [sp ,16 * 13]
        stp x28, x29, [sp ,16 * 14]
        
        mrs x2, sp_el0
        stp x30, x2, [sp, 16 * 15]

        mrs x2, elr_el1
        mrs x3, spsr_el1
        stp x2, x3, [sp, 16 * 16]         // Store elr_el1/spsr_el1 for nested interrupt
        
        ldp x2, x3, [sp ,16 * 1]
    .endm

    // load general registers from stack
    .macro load_all
        ldp x0, x1, [sp ,16 * 0]
        ldp x2, x3, [sp ,16 * 1]
        ldp x4, x5, [sp ,16 * 2]
        ldp x6, x7, [sp ,16 * 3]
        ldp x8, x9, [sp ,16 * 4]
        ldp x10, x11, [sp ,16 * 5]
        ldp x12, x13, [sp ,16 * 6]
        ldp x14, x15, [sp ,16 * 7]
        ldp x16, x17, [sp ,16 * 8]
        ldp x18, x19, [sp ,16 * 9]
        ldp x20, x21, [sp ,16 * 10]
        ldp x22, x23, [sp ,16 * 11]
        ldp x24, x25, [sp ,16 * 12]
        ldp x26, x27, [sp ,16 * 13]
        ldp x28, x29, [sp ,16 * 14]
        
        ldp x30, x2, [sp, 16 * 15]
        msr sp_el0, x2

        ldp x2, x3, [sp, 16 * 16]         // Load elr_el1/spsr_el1 from stack
        msr elr_el1, x2
        msr spsr_el1, x3

        ldp x2, x3, [sp ,16 * 1]

        add sp, sp, 34 * 8
    .endm


// https://github.com/torvalds/linux/blob/master/arch/arm64/kernel/entry.S

    .macro entry_label, el:req, ht:req, regsize:req, label:req
    el\el\ht\()_\regsize\()_\label:
        save_all
        mov	x0, sp
        bl el\el\ht\()_\regsize\()_\label\()_handler
        load_all
        eret
    .endm

    .macro kernel_ventry, el:req, ht:req, regsize:req, label:req
        .align 7
        b el\el\ht\()_\regsize\()_\label
    .endm

    entry_label	1, t, 64, sync		// Synchronous EL1t
    entry_label	1, t, 64, irq		// IRQ EL1t
    entry_label	1, t, 64, fiq		// FIQ EL1t
    entry_label	1, t, 64, error		// Error EL1t

    entry_label	1, h, 64, sync		// Synchronous EL1h
    entry_label	1, h, 64, irq		// IRQ EL1h
    entry_label	1, h, 64, fiq		// FIQ EL1h
    entry_label	1, h, 64, error		// Error EL1h

    entry_label	0, t, 64, sync		// Synchronous 64-bit EL0
    entry_label	0, t, 64, irq		// IRQ 64-bit EL0
    entry_label	0, t, 64, fiq		// FIQ 64-bit EL0
    entry_label	0, t, 64, error		// Error 64-bit EL0

    entry_label	0, t, 32, sync		// Synchronous 32-bit EL0
    entry_label	0, t, 32, irq		// IRQ 32-bit EL0
    entry_label	0, t, 32, fiq		// FIQ 32-bit EL0
    entry_label	0, t, 32, error		// Error 32-bit EL0

.align 11 // vector table should be aligned to 0x800
.global exception_vector_table
exception_vector_table:
	kernel_ventry	1, t, 64, sync		// Synchronous EL1t
	kernel_ventry	1, t, 64, irq		// IRQ EL1t
	kernel_ventry	1, t, 64, fiq		// FIQ EL1t
	kernel_ventry	1, t, 64, error		// Error EL1t

	kernel_ventry	1, h, 64, sync		// Synchronous EL1h
	kernel_ventry	1, h, 64, irq		// IRQ EL1h
	kernel_ventry	1, h, 64, fiq		// FIQ EL1h
	kernel_ventry	1, h, 64, error		// Error EL1h

	kernel_ventry	0, t, 64, sync		// Synchronous 64-bit EL0
	kernel_ventry	0, t, 64, irq		// IRQ 64-bit EL0
	kernel_ventry	0, t, 64, fiq		// FIQ 64-bit EL0
	kernel_ventry	0, t, 64, error		// Error 64-bit EL0

	kernel_ventry	0, t, 32, sync		// Synchronous 32-bit EL0
	kernel_ventry	0, t, 32, irq		// IRQ 32-bit EL0
	kernel_ventry	0, t, 32, fiq		// FIQ 32-bit EL0
	kernel_ventry	0, t, 32, error		// Error 32-bit EL0

.global cpu_switch_to
cpu_switch_to:
    stp x19, x20, [x0, 16 * 0]
    stp x21, x22, [x0, 16 * 1]
    stp x23, x24, [x0, 16 * 2]
    stp x25, x26, [x0, 16 * 3]
    stp x27, x28, [x0, 16 * 4]
    stp fp, lr, [x0, 16 * 5]
    mov x9, sp
    str x9, [x0, 16 * 6]

    ldp x19, x20, [x1, 16 * 0]
    ldp x21, x22, [x1, 16 * 1]
    ldp x23, x24, [x1, 16 * 2]
    ldp x25, x26, [x1, 16 * 3]
    ldp x27, x28, [x1, 16 * 4]
    ldp fp, lr, [x1, 16 * 5]
    ldr x9, [x1, 16 * 6]
    mov sp,  x9
    ret
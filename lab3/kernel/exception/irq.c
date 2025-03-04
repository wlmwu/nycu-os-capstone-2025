#include "irq.h"
#include "bcm2873/reg_interrupt.h"
#include "mini_uart.h"
#include "timer.h"

void irq_enable(){
    __asm__ __volatile__("msr daifclr, 0x2");
}
void irq_disable(){
    __asm__ __volatile__("msr daifset, 0x2");
}

void irq_handle() {
    if ((*IRQ_PENDING_1 & AUX_INT) &&                                                 // AUX interrupt
    (*((volatile unsigned int*)(CORE0_INTERRUPT_SOURCE)) & INTERRUPT_SOURCE_GPU)) {   // GPU interrupt
        // UART interrupt
        // https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf  P. 112
        // https://datasheets.raspberrypi.com/bcm2836/bcm2836-peripherals.pdf  P. 16

        uart_irq_handle();

    } else if ((*((volatile unsigned int*)(CORE0_INTERRUPT_SOURCE)) & INTERRUPT_SOURCE_CNTPNSIRQ)) {    // CNTPNSIRQ interrupt
        // CNTPNSIRQ (Non-Secure Physical Timer Interrupt): Timer interrupt
        // https://datasheets.raspberrypi.com/bcm2836/bcm2836-peripherals.pdf  P. 16

        timer_irq_handle();

    } else {
        uart_printf("IRQ_PENDING_1: %x\n", *IRQ_PENDING_1);
        uart_printf("CORE0_INTERRUPT_SOURCE: %x\n\n", *((volatile unsigned int*)(CORE0_INTERRUPT_SOURCE)));
    }
}


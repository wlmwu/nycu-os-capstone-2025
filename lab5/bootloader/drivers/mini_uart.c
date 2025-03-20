#include "mini_uart.h"

static void delay(unsigned int clock) {
    while (clock--) {
        asm volatile("nop");
    }
}

void uart_init() {
    register unsigned int r;
    
    r = *GPFSEL1;       // Get current state
    r &= ~(7u << 12);   // Clean gpio14
    r |= 2u << 12;      // Set alt5 for gpio14
    r &= ~(7u << 15);   // Clean gpio15
    r |= 2u << 15;      // Set alt5 for gpio 15
    *GPFSEL1 = r;

    *GPPUD = 0;                                // Disable pull-up/down
    delay(150u);                               // Wait 150 cycles
    *GPPUDCLK0 = (1u << 14) | (1u << 15);      // Clock the control signal 
    delay(150u);
    *GPPUDCLK0 = 0u;                           // Remove the clock

    *AUX_ENABLES = 1u;
    *AUX_MU_CNTL_REG = 0u;    
    *AUX_MU_IER_REG = 0u;    
    *AUX_MU_LCR_REG = 3u;    
    *AUX_MU_MCR_REG = 0u;    
    *AUX_MU_BAUD_REG = 270u;
    *AUX_MU_IIR_REG = 6u;
    *AUX_MU_CNTL_REG = 3u;
}

char uart_recv() {
    char c = uart_getc();
    return c == '\r' ? '\n' : c;
}

void uart_putc(const char c) {
    while (!(*AUX_MU_LSR_REG & 0x20));
    *AUX_MU_IO_REG = c;
}

void uart_puts(const char* str) {
    while (*str) {
        if (*str == '\n') uart_putc('\r');
        uart_putc(*str++);
    }
}

char uart_getc() {
    while(!(*AUX_MU_LSR_REG & 0x01));
    char c = (char)(*AUX_MU_IO_REG);
    return c;
}

unsigned int uart_getu() {
    char buf[4];
    
    for (int i = 0; i < 4; ++i) {
        buf[i] = uart_recv();
    }

    return *((unsigned int*)buf);
}
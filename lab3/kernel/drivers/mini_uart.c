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

void uart_putu(const unsigned int num) {
    uart_puts(itos(num, 10));
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

void uart_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%') {
            format++; // Move past '%'

            switch (*format) {
                case 'd': {  // Signed integer
                    int val = va_arg(args, int);
                    if (val < 0) {
                        uart_putc('-');
                        val = -val;
                    }
                    uart_puts(itos((unsigned int)val, 10));
                    break;
                }
                case 'u': {  // Unsigned integer
                    uart_puts(itos(va_arg(args, unsigned int), 10));
                    break;
                }
                case 'x': {  // Hexadecimal
                    uart_puts("0x");
                    uart_puts(itos(va_arg(args, unsigned int), 16));
                    break;
                }
                case 'p': {  // Pointer (Hexadecimal with 0x prefix)
                    uart_puts("0x");
                    uart_puts(itos((unsigned long)va_arg(args, void*), 16));
                    break;
                }
                case 's': {  // String
                    uart_puts(va_arg(args, char*));
                    break;
                }
                case 'c': {  // Character
                    uart_putc((char)va_arg(args, int));
                    break;
                }
                case '%': {  // Literal '%'
                    uart_putc('%');
                    break;
                }
                default: {  // Unknown format specifier, just print it
                    uart_putc('%');
                    uart_putc(*format);
                    break;
                }
            }
        } else {  // Normal character
            if (*format == '\n') uart_putc('\r');  // Convert LF to CRLF
            uart_putc(*format);
        }
        format++;
    }

    va_end(args);
}
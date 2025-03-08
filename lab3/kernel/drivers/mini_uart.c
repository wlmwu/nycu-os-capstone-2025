#include "mini_uart.h"
#include "reg_gpio.h"
#include "reg_uart.h"
#include "reg_interrupt.h"

#include "utils.h"
#include "ringbuffer.h"

#include <stdarg.h>

static ring_buffer_t *uart_rx_buffer;
static ring_buffer_t *uart_tx_buffer;

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

    *ENABLE_IRQS_1 |= (1 << 29);    // Enable AUX int

    static char rxbuf[UART_BUFFER_SIZE];
    static char txbuf[UART_BUFFER_SIZE];
    uart_rx_buffer = ring_buffer_init(rxbuf, UART_BUFFER_SIZE);
    uart_tx_buffer = ring_buffer_init(txbuf, UART_BUFFER_SIZE);
}

char uart_recv() {
    char c = uart_async_getc();
    return c == '\r' ? '\n' : c;
}

void uart_putc(const char c) {
    while (!(*AUX_MU_LSR_REG & 0x20));
    *AUX_MU_IO_REG = c;
}

void uart_puts(const char* str) {
    while (*str) {
        if (*str == '\n') uart_async_putc('\r');
        uart_async_putc(*str++);
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
                        uart_puts("-");
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
                    uart_async_putc((char)va_arg(args, int));
                    break;
                }
                case '%': {  // Literal '%'
                    uart_puts("%");
                    break;
                }
                default: {  // Unknown format specifier, just print it
                    uart_puts("%");
                    uart_async_putc(*format);
                    break;
                }
            }
        } else {  // Normal character
            if (*format == '\n') uart_async_putc('\r');  // Convert LF to CRLF
            uart_async_putc(*format);
        }
        format++;
    }

    va_end(args);
}


void uart_irq_enable() {
    *AUX_MU_IER_REG |= 0b10;        // Enable transmit interrupts
    *AUX_MU_IER_REG |= 0b01;        // Enable receive interrupts
}

void uart_irq_disable() {
    *AUX_MU_IER_REG &= ~(0b10);     // Disable transmit interrupt
    *AUX_MU_IER_REG &= ~(0b01);     // Disable receive interrupt
}

void uart_async_putc(const char c) {
    ring_buffer_enqueue(uart_tx_buffer, c);
    *AUX_MU_IER_REG |= 0b10;        // Enable transmit interrupts
}

char uart_async_getc() {
    char c;
    while (!ring_buffer_dequeue(uart_rx_buffer, &c)) {
        *AUX_MU_IER_REG |= 0b01;        // Enable receive interrupts
    }
    return c;
}

void uart_irq_handle() {
    if(*AUX_MU_IIR_REG & 0b010) {                   // Transmit holding register empty
        while (!ring_buffer_is_empty(uart_tx_buffer)) {
            char c;
            ring_buffer_dequeue(uart_tx_buffer, &c);
            uart_putc(c);
        }
        *AUX_MU_IER_REG &= ~(0b10);  
    } else if(*AUX_MU_IIR_REG & 0b100) {            // Receiver holds valid byte
        if (ring_buffer_is_full(uart_rx_buffer)) {
            *AUX_MU_IER_REG &= ~(0b01);                 // Disable receive interrupt
        } else {
            char c = uart_getc();
            ring_buffer_enqueue(uart_rx_buffer, c);
        }
    } else {
        uart_printf("UART IRQ unknown\n");
    }
}

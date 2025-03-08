#ifndef MINI_UART_H_
#define MINI_UART_H_

#include "reg_uart.h"

#define UART_BUFFER_SIZE 4096

void uart_init();

char uart_recv();
char uart_getc();
unsigned int uart_getu();

void uart_putc(const char c);
void uart_puts(const char* str);
void uart_putu(const unsigned int num);
void uart_printf(const char* format, ...);

char uart_async_getc();
void uart_async_putc(const char c);

void uart_irq_enable();
void uart_irq_disable();
void uart_irq_handle();

#endif // MINI_UART_H_
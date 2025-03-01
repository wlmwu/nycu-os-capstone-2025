#ifndef MINI_UART_H_
#define MINI_UART_H_

#include "bcm2873/reg_gpio.h"
#include "bcm2873/reg_uart.h"
#include "utils.h"

#include <stdarg.h>

void uart_init();

char uart_recv();

char uart_getc();
unsigned int uart_getu();

void uart_putc(const char c);
void uart_puts(const char* str);
void uart_putu(const unsigned int num);

void uart_printf(const char* format, ...);

#endif // MINI_UART_H_
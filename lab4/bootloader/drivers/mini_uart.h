#ifndef MINI_UART_H_
#define MINI_UART_H_

#include "reg_gpio.h"
#include "reg_uart.h"

void uart_init();

char uart_recv();

char uart_getc();
unsigned int uart_getu();

void uart_putc(const char c);
void uart_puts(const char* str);


#endif // MINI_UART_H_
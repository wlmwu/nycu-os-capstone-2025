#ifndef MINI_UART_H_
#define MINI_UART_H_

#include "bcm2837/reg_gpio.h"
#include "bcm2837/reg_uart.h"

void uart_init();
char uart_recv();
void uart_send(const char c);
void uart_puts(const char* str);


#endif // MINI_UART_H_
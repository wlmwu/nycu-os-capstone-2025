#ifndef IO_H_
#define IO_H_

#include "syscall.h"
#include <stdarg.h>

static inline char* itos(unsigned long value, int base) {
    static char buf[32];  // Static buffer to hold the string
    char* ptr = &buf[31]; // Start from the end of the buffer
    *ptr = '\0';          // Null-terminate the string

    if (base < 2 || base > 36) return buf; // Invalid base, return empty string

    // Special case for 0
    if (value == 0) {
        *--ptr = '0';
        return ptr;
    }

    // Convert number to string
    while (value > 0) {
        unsigned long remainder = value % base;
        *--ptr = (remainder < 10) ? '0' + remainder : 'a' + (remainder - 10);
        value /= base;
    }

    return ptr;  // Return pointer to the converted number
}

static inline void puts(char *str) {
    char buf[100];
    char *ptr = buf;
    int len = 0;
    for (len = 0; len < 100 && *str; ++len) {
        ptr[len] = *str++;
    }
    ptr[len] = '\0';

    write(buf, len);
}

static inline void putc(char c) {
    char buf[] = "\0\0";
    buf[0] = c;
    write(buf, 16);
}

static void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    while (*format) {
        if (*format == '%') {
            format++; // Move past '%'

            switch (*format) {
                case 'd': {  // Signed integer
                    int val = va_arg(args, int);
                    if (val < 0) {
                        puts("-");
                        val = -val;
                    }
                    puts(itos((unsigned int)val, 10));
                    break;
                }
                case 'u': {  // Unsigned integer
                    puts(itos(va_arg(args, unsigned int), 10));
                    break;
                }
                case 'x': {  // Hexadecimal
                    puts("0x");
                    puts(itos(va_arg(args, unsigned int), 16));
                    break;
                }
                case 'p': {  // Pointer (Hexadecimal with 0x prefix)
                    puts("0x");
                    puts(itos((unsigned long)va_arg(args, void*), 16));
                    break;
                }
                case 's': {  // String
                    puts(va_arg(args, char*));
                    break;
                }
                case 'c': {  // Character
                    putc((char)va_arg(args, int));
                    break;
                }
                case '%': {  // Literal '%'
                    puts("%");
                    break;
                }
                default: {  // Unknown format specifier, just print it
                    puts("%");
                    putc(*format);
                    break;
                }
            }
        } else {  // Normal character
            if (*format == '\n') putc('\r');  // Convert LF to CRLF
            putc(*format);
        }
        format++;
    }

    va_end(args);
}

static inline char* fgets(char *buf, size_t size) {
    size_t n = 0;
    char c;

    while (n < size - 1) {
        if (read(&c, 1) != 1) break;
        if (c == '\r') c = '\n';
        buf[n++] = c;
        putc(c);
        if (c == '\n') break;
    }

    buf[n] = '\0';
    return (n == 0 && buf[0] == '\0') ? NULL : buf;
}

#endif // IO_H_

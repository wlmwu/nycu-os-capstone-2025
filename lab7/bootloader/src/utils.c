#include "utils.h"

int strcmp(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (*str1 != *str2) {
            return (unsigned char)*str1 - (unsigned char)*str2;
        }
        str1++;
        str2++;
    }

    return (unsigned char)*str1 - (unsigned char)*str2;
}

size_t strlen(const char *str) {
    size_t length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}

void arrset(void *ptr, int value, unsigned int num) {
    unsigned char *p = (unsigned char *)ptr;  // Convert the pointer to a byte-level pointer
    for (unsigned int i = 0; i < num; i++) {
        p[i] = (unsigned char)value;  // Set each byte to the specified value
    }
}

void uint2hexstr(char *output, unsigned int d) {
    for (int i = 7; i >= 0; --i) {
        unsigned char hex_digit = (d >> (i * 4)) & 0xF;
        if (hex_digit < 10) {
            output[7 - i] = '0' + hex_digit;
        } else {
            output[7 - i] = 'a' + (hex_digit - 10);
        }
    }
    output[8] = '\0';
}

char* itos(unsigned long value, int base) {
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

void *memset (void *dest, int val, size_t len) {
    unsigned char *ptr = dest;
    while (len-- > 0)
        *ptr++ = val;
    return dest;
}

uint32_t bswap32(uint32_t value) {
    return ((value & 0xFF000000) >> 24) | 
           ((value & 0x00FF0000) >> 8)  | 
           ((value & 0x0000FF00) << 8)  | 
           ((value & 0x000000FF) << 24);
}
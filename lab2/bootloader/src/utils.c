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
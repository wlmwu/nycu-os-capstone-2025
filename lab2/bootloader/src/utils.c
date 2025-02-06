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

void itos(char* output, unsigned long value, int base) {
    char* ptr = output;
    char* end = output; // To reverse the result later

    if (base < 2 || base > 36) {
        return; // Invalid base
    }

    // Handle 0 as a special case
    if (value == 0) {
        *end++ = '0';
        *end = '\0';
        return;
    }

    // Convert the number to the specified base
    while (value > 0) {
        unsigned long remainder = value % base;
        *end++ = (remainder < 10) ? '0' + remainder : 'a' + (remainder - 10);
        value /= base;
    }

    *end = '\0'; // Null-terminate the string

    // Reverse the string in-place
    char* start = output;
    end--; // Move back to the last character
    while (start < end) {
        char temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}

uint32_t bswap32(uint32_t value) {
    return ((value & 0xFF000000) >> 24) | 
           ((value & 0x00FF0000) >> 8)  | 
           ((value & 0x0000FF00) << 8)  | 
           ((value & 0x000000FF) << 24);
}


/* Reboot */
void set(long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}
void reset(int tick) {                 // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}
void cancel_reset() {
    set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}
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
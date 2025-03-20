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

int strncmp(const char *str1, const char *str2, size_t n) {
    // Compare up to n characters
    while (n-- > 0) {
        // If either string ends or characters differ, return the difference
        if (*str1 != *str2) {
            return (unsigned char)*str1 - (unsigned char)*str2;
        }
        // If both strings end at the same time, return 0
        if (*str1 == '\0') {
            return 0;
        }
        str1++;
        str2++;
    }

    return 0;  // If we reached here, the first n characters are the same
}

void *memset (void *dest, int val, size_t len) {
  unsigned char *ptr = dest;
  while (len-- > 0)
    *ptr++ = val;
  return dest;
}

void *uint2hexstr(char *output, unsigned int d) {
    if (!output) {  // If output is NULL, allocate memory
        output = (char *)malloc(9);
        if (!output) return NULL; // Memory allocation failed
    }
    for (int i = 7; i >= 0; --i) {
        unsigned char hex_digit = (d >> (i * 4)) & 0xF;
        if (hex_digit < 10) {
            output[7 - i] = '0' + hex_digit;
        } else {
            output[7 - i] = 'a' + (hex_digit - 10);
        }
    }
    output[8] = '\0';
    return output;
}

unsigned int hexstr2uint(char *hex) {
    unsigned int result = 0;
    char c;

    for (int i = 0; i < LEN_U32_STR && hex[i] != '\0'; i++) {  // Process exactly 8 chars
        c = hex[i];
        unsigned int digit;

        if (c >= '0' && c <= '9') {
            digit = c - '0';  // Convert '0'-'9' to 0-9
        } else if (c >= 'A' && c <= 'F') {
            digit = c - 'A' + 10;  // Convert 'A'-'F' to 10-15
        } else if (c >= 'a' && c <= 'f') {
            digit = c - 'a' + 10;  // Convert 'a'-'f' to 10-15
        } else {
            break;  // Stop on invalid character
        }

        result = (result << 4) | digit;  // Shift left 4 bits and add new value
    }

    return result;
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

int stoi(const char *str, char **endptr, int base) {
    int result = 0;
    int sign = 1;

    // Skip leading whitespaces
    while (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r') {
        str++;
    }

    // Handle optional '+' or '-'
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+') {
        str++;
    }

    // Convert digits
    while (*str >= '0' && *str <= '9') {
        result = result * base + (*str - '0');
        str++;
    }

    // Set endptr if needed
    if (endptr) {
        *endptr = (char *)str;
    }

    return result * sign;
}

char *strchr(const char *str, char c) {
    while (*str) {
        if (*str == c) {
        return (char *)str; // Return a pointer to the found character
        }
        str++;
    }
    return NULL; // Return NULL if the character is not found
}

char *strtok(char *str, const char *delim) {
    static char *last; // Remember where we left off in the string
    char *start; // The beginning of the current token

    // If the input string is NULL, continue from where we left off
    if (str != NULL) {
        last = str;
    }

    // If there is no string left to process
    if (last == NULL) {
        return NULL;
    }

    // Skip leading delimiters
    while (*last && strchr(delim, *last)) {
        last++;
    }

    // If we reached the end of the string
    if (*last == '\0') {
        return NULL;
    }

    // Set the start of the token
    start = last;

    // Find the next delimiter or end of string
    while (*last && !strchr(delim, *last)) {
        last++;
    }

    // If we found a delimiter, terminate the token there
    if (*last != '\0') {
        *last = '\0';
        last++; // Move past the delimiter for the next call
    }

    return start;
}

size_t strlen(const char *str) {
    size_t length = 0;
    while (str[length] != '\0') {
        length++;
    }
    return length;
}

int split_args(char* buf, char* argv[], int max_args) {
    int argc = 0;
    char* token = strtok(buf, " "); // Split by space
    while (token != NULL && argc < max_args) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    return argc;
}

extern char _heap_start;  // Start of the heap
extern char _heap_end;    // End of the heap
static char* alloc_ptr_start = &_heap_start;  // Start of the heap region
void* malloc(size_t size) {
    // Ensure the allocation is aligned (e.g., 8-byte alignment)
    size = (size + 7) & ~7;

    if (alloc_ptr_start + size > &_heap_end) {
        return NULL;  // Out of memory
    }

    void* ptr = alloc_ptr_start;
    alloc_ptr_start += size;  // Move the pointer forward by the allocated size
    return ptr;
}
void free(void *ptr) { /*Unimplemented*/ }

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
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
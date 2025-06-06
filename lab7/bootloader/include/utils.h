#ifndef UTILS_H_
#define UTILS_H_

#include <stddef.h>
#include <stdint.h>

/**
 * @param dest   - Pointer to the memory block to be filled.
 * @param val - Value to be set. Only the least significant byte is used.
 * @param len   - Number of bytes to set to the specified value.
 * 
 * @return void This function does not return any value.
*/
void *memset (void *dest, int val, size_t len);

/*
 * @param str1 A pointer to the first null-terminated string.
 * @param str2 A pointer to the second null-terminated string.
 *
 * @return An integer less than, equal to, or greater than zero, depending on whether `str1` is lexicographically
 *         less than, equal to, or greater than `str2`.
 *         - Negative value: `str1` is less than `str2`
 *         - Zero: `str1` is equal to `str2`
 *         - Positive value: `str1` is greater than `str2`
*/
int strcmp(const char *str1, const char *str2);

/**
 * Converts an unsigned long integer to a string in the specified base.
 * 
 * @param value The unsigned long integer to convert.
 * @param base  The numeric base for conversion (e.g., 10 for decimal, 16 for hexadecimal).
 *              Valid values are between 2 and 36.
 * 
 * @return A pointer to a static character buffer containing the converted string.
 *         The buffer is overwritten on each call, so its contents should be used immediately.
 */
char* itos(unsigned long value, int base);

#endif // UTILS_H_
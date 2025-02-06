#ifndef UTILS_H_
#define UTILS_H_

#include <stddef.h>
#include <stdint.h>

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

size_t strlen(const char *str);

/*
 * @param ptr   - Pointer to the memory block to be filled.
 * @param value - Value to be set. Only the least significant byte is used.
 * @param num   - Number of bytes to set to the specified value.
 * 
 * @return void This function does not return any value.
*/
void arrset(void *ptr, int value, unsigned int num);

/*
 * @param output A pointer to a character array where the hexadecimal string will be stored.
 *               The array should be large enough to store 8 characters and the null terminator.
 * @param d The unsigned integer to be converted to a hexadecimal string.
 * 
 * @return void This function does not return a value, it directly modifies the `output` array.
*/
void uint2hexstr(char *output, unsigned int d);
#define LEN_U32_HEX_STR 11

/*
 * @param output A pointer to a character array where the converted string will be stored.
 *               The array should be large enough to store the digits of the number, 
 *               the null terminator, and possible characters for the base.
 * @param value  The unsigned long integer to be converted to a string.
 * @param base   The numeric base for the conversion (e.g., 10 for decimal, 16 for hexadecimal).
 *               Valid values are between 2 and 36.
 * 
 * @return void This function does not return a value, it directly modifies the `output` array.
 */
void itos(char* output, unsigned long value, int base);

#define ALIGN(val, align_len) (((val) + (align_len) - 1) / (align_len) * (align_len))

uint32_t bswap32(uint32_t value);

/* Reboot */
#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

#define NUM_TICKS 100

void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();


#endif // UTILS_H_
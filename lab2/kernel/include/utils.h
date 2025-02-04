#ifndef UTILS_H_
#define UTILS_H_

#include <stddef.h>

/**
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

int strncmp(const char *str1, const char *str2, size_t n);

/**
 * @brief Finds the first occurrence of a character in a string.
 * 
 * Searches for the character `c` in the string `str` and returns a pointer to its first occurrence.
 * 
 * @param str A pointer to the null-terminated string where the search will be performed.
 * @param c   The character to be searched for in the string `str`.
 * 
 * @return char* A pointer to the first occurrence of the character `c` in the string `str`,
 *               or NULL if the character `c` is not found.
 */
char *strchr(const char *str, char c);

/**
 * @brief Splits a string into tokens based on delimiters.
 * 
 * This function tokenizes a string by splitting it into substrings, using the characters in `delim` as delimiters.
 * 
 * @param str   A pointer to the string to be tokenized. On subsequent calls, pass NULL to continue tokenizing.
 * @param delim A pointer to a string containing the delimiters.
 * 
 * @return char* A pointer to the next token in the string, or NULL if there are no more tokens.
 */
char *strtok(char *str, const char *delim);

/**
 * @brief Splits a string into individual arguments (tokens).
 * 
 * This function tokenizes the input string `buf` and stores the resulting arguments in the `argv` array.
 * It uses `strtok` internally to split the string.
 * 
 * @param buf       A pointer to the input string to be split.
 * @param argv      A pointer to an array where the split arguments will be stored.
 * @param max_args  The maximum number of arguments that can be stored in `argv`.
 * 
 * @return int      The number of arguments found and stored in the `argv` array.
 */
int split_args(char* buf, char* argv[], int max_args);

/**
 * @param ptr   - Pointer to the memory block to be filled.
 * @param value - Value to be set. Only the least significant byte is used.
 * @param num   - Number of bytes to set to the specified value.
 * 
 * @return void This function does not return any value.
*/
void arrset(void *ptr, int value, unsigned int num);

/**
 * @param output A pointer to a character array where the hexadecimal string will be stored.
 *               The array should be large enough to store 8 characters and the null terminator.
 * @param d The unsigned integer to be converted to a hexadecimal string.
 * 
 * @return void This function does not return a value, it directly modifies the `output` array.
*/
void uint2hexstr(char *output, unsigned int d);
#define LEN_U32_HEX_STR 11

/**
 * @param hex A pointer to an array containing 8 hexadecimal digits ('0'-'9', 'A'-'F', 'a'-'f').
 * 
 * @return The corresponding unsigned integer value of the hexadecimal string.
 */
unsigned int hexstr2uint(char *hex);
#define LEN_U32_STR 8

/**
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

/* Reboot */
#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

#define NUM_TICKS 100

void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();


#endif // UTILS_H_
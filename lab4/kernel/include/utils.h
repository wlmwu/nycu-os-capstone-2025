#ifndef UTILS_H_
#define UTILS_H_

#include <stddef.h>
#include <stdint.h>

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
 * @brief Computes the length of a null-terminated C string.
 *
 * @param str A pointer to a null-terminated C string (const char*).
 *            The string must be properly null-terminated to avoid undefined behavior.
 *
 * @return The length of the string, as a size_t value, which represents the number
 *         of characters before the null terminator.
 *
 * @note The string must be null-terminated; passing a non-null-terminated string
 *       will lead to undefined behavior.
 */
size_t strlen(const char *str);

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
 * Converts an unsigned integer to a hexadecimal string.
 * 
 * @param output A pointer to a character array where the hexadecimal string will be stored.
 *               If NULL is passed, the function will allocate memory dynamically.
 * @param d The unsigned integer to be converted to a hexadecimal string.
 * 
 * @return A pointer to the hexadecimal string.
 *         If `output` is provided, it returns the same pointer.
 *         If `output` is NULL, it returns a dynamically allocated string, 
 *         which must be freed by the caller.
 *         Returns NULL if memory allocation fails.
 */
void *uint2hexstr(char *output, unsigned int d);
#define LEN_U32_HEX_STR 11

/**
 * @param hex A pointer to an array containing 8 hexadecimal digits ('0'-'9', 'A'-'F', 'a'-'f').
 * 
 * @return The corresponding unsigned integer value of the hexadecimal string.
 */
unsigned int hexstr2uint(char *hex);
#define LEN_U32_STR 8

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

/**
 * Convert a string to an integer.
 * 
 * @param str: The string to convert.
 * @param endptr: Pointer to store the address of the first invalid character (can be NULL).
 * @param base: The numerical base (e.g., 10 for decimal, 16 for hexadecimal).
 *
 * @return The converted integer value.
 */
int stoi(const char *str, char **endptr, int base);

void* malloc(size_t size);
void free(void *ptr);

/**
 * Copy memory area. Copies `n` bytes from memory area `src` to memory area `dest`.
 * The memory areas **must not overlap**.
 * 
 * @param dest: Destination memory area
 * @param src: Source memory area
 * @param n: Number of bytes to copy
 *
 * @return the original value of `dest`.
 */
void *memcpy(void *dest, const void *src, size_t n);

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
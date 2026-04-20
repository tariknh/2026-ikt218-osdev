#pragma once
#include <stdint.h>

/**
 * Count the length of a string
 * @param str string
 * @return length of string
 */
size_t strlen(const char* str);

/**
 * Check if two strings are identical
 * @param l string
 * @param r string
 * @return 0 if identical
 */
int strcmp(const char* l, const char* r);

/**
 * Compares up to num characters of c strings l and r.
 * @param l string
 * @param r string
 * @param n maximum number of characters to compare
 * @return 0 if equal, else l < r
 */
int strncmp(const char* l, const char* r, size_t n);

/**
 * Copy memory from one location to another
 * @param dest destination memory address
 * @param src source memory address
 * @param count bytes to copy
 * @return destination memory address
 */
void* memcpy(void* restrict dest, const void* restrict src, size_t count);

/**
 * Sets memory in destination memory to supplied byte
 * @param dest destination memory address
 * @param ch byte to set
 * @param count number of bytes to set
 * @return destination memory address
 */
void* memset(void* dest, int ch, size_t count);

/**
 * String copy
 * @param dest string to set
 * @param src string source
 * @return new string pointer
 */
char* strcpy(char* dest, const char* src);

/**
 * String copy with max charactes to copy
 * @param dest string to  set
 * @param src string source
 * @param count max size
 * @return new string pointer
 */
char* strncpy(char* dest, const char* src, size_t count);

/**
 * Returns a pointer to the last occurrence of character in the C string str.
 * @param s string
 * @param c character
 * @return pointer to last character in string or NULL
 */
char* strrchr (const char* s, int c);

/**
 * Appends the first num characters of source to destination, plus a terminating null-character.
 * @param dest
 * @param src C string to be appended.
 * @param n Maximum number of characters to be appended.
 * @return destination is returned.
 */
char* strncat (char* dest, const char* src, size_t n);
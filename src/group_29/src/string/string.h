#pragma once

#include "libc/stdint.h"
#include "libc/stddef.h"

#include "../memory/heap.h"


/// @brief Finds the length of a string
///
/// @param string Pointer to a null-terminated string.
///
/// @returns The length of the string
int32_t strlen(const char string[]);


/// @brief Compares two null-terminated strings lexicographically.
///
/// Iterates through both strings character by character until a difference
/// is found or a null terminator is reached.
///
/// @param string_a Pointer to the first null-terminated string.
/// @param string_b Pointer to the second null-terminated string.
///
/// @returns 0 if both strings are equal, < 0 if the first differing character in string_a is less than in string_b, > 0 if the first differing character in string_a is greater than in string_b
///
/// @warning The function assumes both pointers are valid and point to null-terminated strings. Passing NULL or non-terminated strings results in undefined behavior.
int16_t strcmp(const char string_a[], const char string_b[]);


/// @brief Converts a 32-bit signed integer to its decimal string representation.
///
/// Writes the ASCII representation of the given integer into the provided buffer.
/// The function handles negative numbers (including INT32_MIN) and does not append
/// a null terminator.
///
/// @param value The 32-bit signed integer to convert.
/// @param buf Pointer to a character buffer where the resulting digits will be written.
///
/// @returns The number of characters written to the buffer (excluding any null terminator).
///
/// @warning The buffer must be large enough to hold the result (up to 11 characters for "-2147483648"). The output is NOT null-terminated; you must append '\0' manually if needed. Passing a NULL buffer results in undefined behavior.
int int_to_str(int32_t value, char* buf);


/// @brief Converts a string to a 32-bit signed integer with validation.
///
/// Parses a null-terminated string into an integer. The function supports
/// optional leading whitespace and an optional '+' or '-' sign. It validates
/// that the entire string is a valid integer representation and rejects
/// malformed inputs.
///
/// @param str Pointer to a null-terminated string containing the number.
/// @param ok Output flag set to 1 if conversion succeeded, or 0 if invalid input.
///
/// @returns The parsed 32-bit signed integer. Returns 0 if parsing fails.
///
/// @warning If `ok` is NULL, no success/failure information will be returned. The function does not currently detect integer overflow.
int32_t str_to_int_checked(const char str[], int* ok);


/// @brief Adds a number to a string by using '%d' as placeholder.
///
/// @param input_string A pointer to the start of the string you want to format. Include '%d' as a placeholder for the value.
/// @param value The value you want to include in the string. Use '%d' to specify where it should be.
///
/// @returns Pointer to a malloc'd char array with the formatted string.
/// @warning Remember to free() the char array pointer after use.
char* format_string(char input_string[], int32_t value);


/// @brief Copies a null-terminated string into a fixed-size buffer.
///
/// Copies `input` into `output` if it fits within `output_length`.
///
/// @param input Source null-terminated string.
/// @param output Destination buffer.
/// @param output_length Size of the destination buffer.
///
/// @return 0 on success, -1 if the input string does not fit.
int16_t string_copy(const char input[], char output[], int16_t output_length);
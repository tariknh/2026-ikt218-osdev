#include "libc/stdint.h"
#include "libc/stddef.h"

int32_t strlen(const char string[]);
static int int_to_str(int32_t value, char* buf);

/// @brief Adds a number to a string by using '%d' as placeholder.
///
/// @param input_string A pointer to the start of the string you want to format. Include '%d' as a placeholder for the value.
/// @param value The value you want to include in the string. Use '%d' to specify where it should be.
///
/// @returns Pointer to a malloc'd char array with the formatted string.
/// @warning Remember to free() the char array pointer after use!
char* format_string(char input_string[], int32_t value);

#include "libc/stdint.h"
#include "libc/stddef.h"

static int int_to_str(int32_t value, char* buf);

/// @brief Formats a string with an int32_t
///
/// @param input_string A pointer to the start of the string you want to format. Include '%d' as a placeholder for the value.
/// @param input_size Size of the input char array.
/// @param value The value you want to include in the string. Use '%d' to specify where it should be.
///
/// @returns Pointer to a malloc'd char array with the formatted string.
/// @warning Remember to free() the char array pointer after use!
const char* format_string(char input_string[], uint16_t input_size, int32_t value);
#pragma once
#include "libc/stddef.h"
#include "libc/stdint.h"

void terminal_initialize(void);
void terminal_put_char(char c);
void terminal_print_string(const char* str);

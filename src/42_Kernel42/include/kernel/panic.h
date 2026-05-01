#pragma once
#include "log.h"


/**
 * Halts the kernel indefinitely after printing a message
 * @param fmt format string
 */
#define kernel_panic(fmt, ...) kernel_panic_impl(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

void kernel_panic_impl(const char* file, int line, const char* fmt, ...);

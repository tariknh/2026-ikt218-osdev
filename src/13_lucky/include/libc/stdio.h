#pragma once
#include "cdefs.h"

EXTERN_C_BEGIN

int putchar(int c);

int printf(const char *__restrict__ format, ...);

void terminal_clear(void);

EXTERN_C_END

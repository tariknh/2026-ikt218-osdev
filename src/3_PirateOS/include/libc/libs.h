#pragma once

// Convenience header that pulls in the small libc-style helpers used by the kernel

#include "libc/limits.h"
#include "libc/stdarg.h"
#include "libc/stdbool.h"
#include "libc/stddef.h"
#include "libc/stdint.h"
#include "libc/memory.h"
#include "libc/stdio.h"
#include "libc/string.h"

#define EOF (-1)

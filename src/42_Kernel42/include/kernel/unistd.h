#pragma once
#include <stdint.h>

/*
 * FILE SEEK FLAGS
 * Values match Linux/i386 unistd.h.
 * Source: https://github.com/torvalds/linux/blob/master/include/uapi/asm-generic/unistd.h*
 */

#define SEEK_SET 0 /* Seek from beginning of file */
#define SEEK_CUR 1 /* Seek from current position */
#define SEEK_END 2 /* Seek from end of file */

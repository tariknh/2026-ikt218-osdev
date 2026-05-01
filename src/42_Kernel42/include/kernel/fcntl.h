#pragma once
#include <stdint.h>

/*
 * FILE OPEN FLAGS
 * Values match Linux/i386 fcntl.h.
 * Source: https://github.com/torvalds/linux/blob/master/include/uapi/asm-generic/fcntl.h
 */

#define O_RDONLY 0  // READONLY
#define O_WRONLY 1  // Open for WRITE ONLY
#define O_RDWR 2    // Open or READ/WRITE
#define O_CREAT 64  // CREATE, fail if exists
#define O_APPEND 8A // If exists appends to file
#define O_EXCL 128  // Error if create and file exists
#define O_TRUNC 512 // If exists truncate file to length 0

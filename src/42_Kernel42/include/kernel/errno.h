#pragma once
#include <stdint.h>

/*
 * ERROR CODES
 * Values match Linux/i386 convention
 * From https://github.com/torvalds/linux/blob/master/include/uapi/asm-generic/errno.h
 *         https://github.com/torvalds/linux/blob/master/include/uapi/asm-generic/errno-base.h
 */

#define ENOSYS 38 /* Invalid syscall number */
#define EBADF 9   /* Bad file descriptor */
#define EINVAL 22 /* Invalid argument */
#define ENOENT 2  /* No such file or directory */
#define EACCES 13 /* Permission denied */

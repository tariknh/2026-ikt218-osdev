#pragma once
#include "libc/stdarg.h"
#include "libc/stdbool.h"
#include "libc/stdint.h"

#define BUFSIZ 1024
typedef struct _IO_FILE FILE;

/// Standard Input
#define stdin (stdin) // file descriptor 0
/// Standard Output
#define stdout (stdout) // file descriptor 1
/// Standard Error Output
#define stderr (stderr) // file decriptor 2

#define _IO_READ 1
#define _IO_WRITE 2
#define _IO_UNBUFFERED 4
#define _IO_EOF_SEEN 8
#define _IO_ERR_SEEN 16

#define EOF (-1)

extern FILE* const stdin;
extern FILE* const stdout;
extern FILE* const stderr;

/// Writes any unwritten data from streams buffer to output device.
/// @param stream stream to flush
/// @return 0 on sucess, otherwise EOF and error indicator of file stream is set.
int fflush(FILE* stream);

/// @param c char to write
/// @param stream stream to write to
/// @return char written
int fputc(int c, FILE* stream);

/// @param ic  char to write
/// @return  char written
int putchar(int ic);

/// @param format format string
/// @param ... arguments
/// @return number of characters written
int printf(const char* restrict format, ...);

/// @param stream stream to write to
/// @param format format string
/// @param ... arguments
/// @return number of characters written
int fprintf(FILE* restrict stream, const char* restrict format, ...);

/// @param stream stream to write to
/// @param format format string
/// @param arg variable length arguments
/// @return number of characters written
int vfprintf(FILE* stream, const char* format, va_list arg);

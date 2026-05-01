#include "libc/stdio.h"

#include <kernel/util.h>

#include "drivers/video/vga_text.h"
#include "libc/stdarg.h"
#include "libc/stddef.h"
#include "libc/stdint.h"


typedef struct _IO_FILE {
  int fd; // file descriptor
  int flags;
  char* buf;
  char* rpos; // Read position
  char* wpos; // Write position
} _IO_FILE;

// bss is automatically zero init
static char stdin_buf[BUFSIZ];
static char stdout_buf[BUFSIZ];


static _IO_FILE __stdin = {
    .fd = 0,
    .flags = _IO_READ,
    .buf = stdin_buf,

};

static _IO_FILE __stdout = {
    .fd = 1,
    .flags = _IO_WRITE,
    .buf = stdout_buf,

};

static _IO_FILE __stderr = {
    .fd = 2,
    .flags = _IO_WRITE | _IO_UNBUFFERED,
    .buf = NULL,

};

// Set initalized instances of the _IO_FILE's
FILE* const stdin = &__stdin;
FILE* const stdout = &__stdout;
FILE* const stderr = &__stderr;

int fflush(FILE* stream) {
  if (!stream->buf || stream->wpos == stream->buf)
    return EOF;

  vga_text_write(stream->buf, stream->wpos - stream->buf);

  stream->wpos = stream->buf;
  return 0;
}

int putchar(int c) {
  return fputc(c, stdout);
}

int fputc(int c, FILE* stream) {
  // If stream is unbuffered, just directly print it, for each char.
  if (!stream->buf || (stream->flags & _IO_UNBUFFERED)) {
    if (stream == stderr) {
      vga_text_setcolor(vga_text_entry_color(VGA_COLOR_RED, VGA_COLOR_BLACK));
      vga_text_putchar((char)c);
      vga_text_setcolor(vga_text_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
      return c;
    }
    vga_text_putchar((char)c);
    return c;
  }
  // if write pos is NULL, set it to the start
  if (!stream->wpos) {
    stream->wpos = stream->buf;
  }

  *stream->wpos++ = (char)c;

  if (c == '\n' || (stream->wpos - stream->buf) >= BUFSIZ) {
    fflush(stream);
  }
  return c;
}

int fprintf(FILE* restrict stream, const char* restrict format, ...) {
  va_list args;
  va_start(args, format);
  int ret = vfprintf(stream, format, args);
  va_end(args);
  return ret;
}
int printf(const char* restrict format, ...) {
  va_list args;
  va_start(args, format);

  int ret = vfprintf(stdout, format, args);

  va_end(args);
  return ret;
}
static void print_dec(long value, FILE* stream, int* count) {
  if (value == 0) {
    fputc('0', stream);
    (*count)++;
    return;
  }
  if (value < 0) {
    fputc('-', stream);
    (*count)++;
    value = -value;
  }
  char buf[20];
  int idx = 0;
  while (value > 0) {
    buf[idx++] = '0' + (value % 10);
    value /= 10;
  }
  while (idx--) {
    fputc(buf[idx], stream);
    (*count)++;
  }
}

static void print_udec(uint32_t value, FILE* stream, int* count) {
  if (value / 10) {
    print_udec(value / 10, stream, count);
  }
  fputc(value % 10 + '0', stream);
  (*count)++;
}

static void print_udec64(uint64_t n, FILE* stream, int* printed_chars) {
  if (n == 0) {
    fputc('0', stream);
    (*printed_chars)++;
    return;
  }
  char buf[20];
  int idx = 0;
  while (n > 0) {
    uint32_t remainder = div_u64_by_10(&n); // Updates n, returns digit
    buf[idx++] = '0' + remainder;
  }
  while (idx--) {
    fputc(buf[idx], stream);
    (*printed_chars)++;
  }
}


static void print_hex64(uint64_t n, FILE* stream, int* printed_chars, int uppercase) {
  if (n == 0) {
    fputc('0', stream);
    (*printed_chars)++;
    return;
  }
  char buf[16];
  int idx = 0;
  while (n > 0) {
    uint32_t digit = n & 0xF;
    buf[idx++] = (digit < 10 ? '0' + digit : (uppercase ? 'A' : 'a') + digit - 10);
    n >>= 4;
  }
  while (idx--) {
    fputc(buf[idx], stream);
    (*printed_chars)++;
  }
}

static void print_hex(unsigned long value, FILE* stream, int* count) {
  if (value / 16) {
    print_hex(value / 16, stream, count);
  }

  unsigned long digit = value % 16;
  char c = (char)(digit < 10 ? digit + '0' : digit - 10 + 'a');
  fputc(c, stream);
  (*count)++;
}


void print_dec64(int64_t n, FILE* stream, int* printed_chars) {
  if (n < 0) {
    fputc('-', stream);
    (*printed_chars)++;
    n = -n;
  }
  print_udec64((uint64_t)n, stream, printed_chars);
}

int vfprintf(FILE* stream, const char* format, va_list arg) {
  int printed_chars = 0;

  for (size_t i = 0; format[i] != '\0'; i++) {
    if (format[i] != '%') {
      fputc(format[i], stream);
      printed_chars++;
      continue;
    }

    i++; // skip '%'

    // Check for minus flag (left alignment)
    int left_align = 0;
    if (format[i] == '-') {
      left_align = 1;
      i++;
    }

    // Check for dynamic width (e.g., %*s) or static width (e.g., %10s)
    int width = 0;
    if (format[i] == '*') {
      // Get width from va_arg
      width = va_arg(arg, int);
      i++;
    } else if (format[i] >= '0' && format[i] <= '9') {
      // Parse static width
      while (format[i] >= '0' && format[i] <= '9') {
        width = width * 10 + (format[i] - '0');
        i++;
      }
    }

    // Length modifier
    int length = 0; // 0: default, 1: l, 2: ll
    if (format[i] == 'l') {
      length = 1;
      i++;
      if (format[i] == 'l') {
        length = 2;
        i++;
      }
    }

    switch (format[i]) {
      case 'c': {
        char c = (char)va_arg(arg, int);
        fputc(c, stream);
        printed_chars++;
        break;
      }
      case 's': {
        const char* s = va_arg(arg, const char*);
        if (!s)
          s = "(null)";

        // Calculate string length for padding
        int len = 0;
        const char* tmp = s;
        while (*tmp++) len++;

        // If left aligned, print string first then padding
        if (left_align) {
          while (*s) {
            fputc(*s++, stream);
            printed_chars++;
          }
          // Pad on right
          if (width > len) {
            int pad = width - len;
            for (int p = 0; p < pad; p++) {
              fputc(' ', stream);
              printed_chars++;
            }
          }
        } else {
          // Print left padding if width > string length
          if (width > len) {
            int pad = width - len;
            for (int p = 0; p < pad; p++) {
              fputc(' ', stream);
              printed_chars++;
            }
          }
          while (*s) {
            fputc(*s++, stream);
            printed_chars++;
          }
        }
        break;
      }
      case 'd':
      case 'i': {
        if (length == 2) {
          int64_t d = va_arg(arg, int64_t);
          print_dec64(d, stream, &printed_chars); // See helpers below
        } else if (length == 1) {
          long d = va_arg(arg, long);
          print_dec((int32_t)d, stream, &printed_chars); // Reuse 32-bit
        } else {
          int d = va_arg(arg, int);
          print_dec(d, stream, &printed_chars);
        }
        break;
      }
      case 'u': {
        if (length == 2) {
          uint64_t u = va_arg(arg, uint64_t);
          print_udec64(u, stream, &printed_chars);
        } else {
          unsigned u = va_arg(arg, unsigned);
          print_udec((uint32_t)u, stream, &printed_chars);
        }
        break;
      }
      case 'x':
      case 'X': {
        int uppercase = (format[i] == 'X');
        if (length == 2) {
          uint64_t x = va_arg(arg, uint64_t);
          print_hex64(x, stream, &printed_chars, uppercase);
        } else {
          unsigned long x = va_arg(arg, unsigned long);
          print_hex((uint32_t)x, stream, &printed_chars);
        }
        break;
      }
      case 'p': {
        uintptr_t p = (uintptr_t)va_arg(arg, void*);
        fputc('0', stream);
        fputc('x', stream);
        printed_chars += 2;
        print_hex((uint32_t)p, stream, &printed_chars); // 32-bit limit OK for pointers?
        break;
      }
      case '%': {
        fputc('%', stream);
        printed_chars++;
        break;
      }
      default:
        fputc('%', stream);
        fputc(format[i], stream);
        printed_chars += 2;
        break;
    }
  }
  return printed_chars;
}

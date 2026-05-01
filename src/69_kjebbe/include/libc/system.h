#pragma once

#include "../../include/libc/stdio.h"

// Kernel panic message
static inline void panic(const char *msg) {
  printf("KERNEL PANIC: ");
  printf(msg);
  asm volatile("cli");
  asm volatile("hlt");
}

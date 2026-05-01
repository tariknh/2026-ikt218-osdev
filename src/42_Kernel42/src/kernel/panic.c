#include "kernel/panic.h"

#include "drivers/video/vga_text.h"


void kernel_panic_impl(const char* file, int line, const char* fmt, ...) {
  // Log location via logger (if still functional)
  log_printf(LOG_LEVEL_FATAL, LOG_SUBSYS_PANIC, "Panic in %s:%d\n", file, line);
  va_list ap;
  va_start(ap, fmt);
  log_vprintf(LOG_LEVEL_FATAL, LOG_SUBSYS_PANIC, fmt, ap);
  va_end(ap);

  vga_text_writestring("\nSystem halted.");
  vga_text_disable_cursor();

  while (true) {
    __asm__ volatile("cli");
    __asm__ volatile("hlt");
  }
}

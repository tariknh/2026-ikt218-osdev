#include "kernel/log.h"

#include "drivers/video/vga_text.h"

static log_level_t g_min_level = LOG_LEVEL_INFO;

static const char* g_level_tags[LOG_LEVEL_COUNT] = {
    [LOG_LEVEL_FATAL] = "PANIC", [LOG_LEVEL_ERROR] = "ERROR", [LOG_LEVEL_WARN] = "WARN ",
    [LOG_LEVEL_INFO] = "INFO ",  [LOG_LEVEL_DEBUG] = "DEBUG", [LOG_LEVEL_TRACE] = "TRACE"};

static const char* g_subsys_names[LOG_SUBSYS_COUNT] = {[LOG_SUBSYS_KERNEL] = "",
                                                       [LOG_SUBSYS_MM] = "MM   ",
                                                       [LOG_SUBSYS_VGA] = "VGA  ",
                                                       [LOG_SUBSYS_IRQ] = "IRQ  ",
                                                       [LOG_SUBSYS_PANIC] = ""};

static uint8_t level_to_color(const log_level_t level) {
  switch (level) {
    case LOG_LEVEL_FATAL:
      return vga_text_entry_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    case LOG_LEVEL_ERROR:
      return vga_text_entry_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    case LOG_LEVEL_WARN:
      return vga_text_entry_color(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK);
    case LOG_LEVEL_INFO:
      return vga_text_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    case LOG_LEVEL_DEBUG:
      return vga_text_entry_color(VGA_COLOR_LIGHT_BLUE, VGA_COLOR_BLACK);
    case LOG_LEVEL_TRACE:
      return vga_text_entry_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    default:
      return vga_text_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
  }
}

void log_init(void) {
  g_min_level = LOG_LEVEL_MAX;
}

void log_set_min_level(log_level_t level) {
  g_min_level = level;
}

static int log_vprintf_internal(log_level_t level, log_subsys_t subsys, const char* fmt,
                                va_list ap) {
  if (level > g_min_level)
    return 0;

  // store old color to restore
  uint8_t old_color = vga_text_get_color();
  vga_text_setcolor(level_to_color(level));

  fprintf(stdout, "[%s] %s: ", g_level_tags[level] ? g_level_tags[level] : "UNKNOWN SUBSYSTEM  ",
          g_subsys_names[subsys] ? g_subsys_names[subsys] : "UNKNOWN SUBSYSTEM  ");

  int chars = vfprintf(stdout, fmt, ap);

  // restore old color
  vga_text_setcolor(old_color);
  fflush(stdout);
  return chars;
}

int log_vprintf(log_level_t level, log_subsys_t subsys, const char* fmt, va_list ap) {
  return log_vprintf_internal(level, subsys, fmt, ap);
}

int log_printf(log_level_t level, log_subsys_t subsys, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = log_vprintf_internal(level, subsys, fmt, ap);
  va_end(ap);
  return ret;
}

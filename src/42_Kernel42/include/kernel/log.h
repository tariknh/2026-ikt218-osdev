#pragma once
#include "libc/stdarg.h"
#include "libc/stdint.h"
#include "libc/stdio.h"

typedef enum {
  LOG_LEVEL_FATAL = 0,
  LOG_LEVEL_ERROR = 1,
  LOG_LEVEL_WARN = 2,
  LOG_LEVEL_INFO = 3,
  LOG_LEVEL_DEBUG = 4,
  LOG_LEVEL_TRACE = 5,
  LOG_LEVEL_COUNT
} log_level_t;

typedef enum {
  LOG_SUBSYS_KERNEL,
  LOG_SUBSYS_MM,
  LOG_SUBSYS_VGA,
  LOG_SUBSYS_IRQ,
  LOG_SUBSYS_PANIC,
  LOG_SUBSYS_COUNT
} log_subsys_t;

#ifndef LOG_LEVEL_MAX
#define LOG_LEVEL_MAX LOG_LEVEL_INFO
#endif

#ifndef ENABLE_LOG_SUBSYS
#define ENABLE_LOG_SUBSYS 1
#endif

#ifndef CURRENT_LOG_SUBSYS
#define CURRENT_LOG_SUBSYS LOG_SUBSYS_KERNEL
#endif

/**
 * Initializes the logger to only emit logs up to the set LOG_LEVEL_MAX
 */
void log_init(void);
/**
 * Set the level of logging
 * @param level log_level_t
 */
void log_set_min_level(log_level_t level);

/**
 * Enable logging for a subsystem
 * @param subsys subsystem
 */
#define _LOG_SUBSYS_EN(subsys) ENABLE_LOG_SUBSYS

/**
 * Log something
 */
#define log_printf_safe(level, subsys, fmt, ...)              \
  do {                                                        \
    if ((level) <= LOG_LEVEL_MAX && _LOG_SUBSYS_EN(subsys)) { \
      log_printf((level), (subsys), fmt, ##__VA_ARGS__);      \
    }                                                         \
  } while (0)

#define log_printf_safe_context(level, fmt, ...) \
  log_printf_safe(level, CURRENT_LOG_SUBSYS, fmt, ##__VA_ARGS__)

#define log_fatal(...) log_printf_safe_context(LOG_LEVEL_FATAL, ##__VA_ARGS__)
#define log_error(...) log_printf_safe_context(LOG_LEVEL_ERROR, ##__VA_ARGS__)
#define log_warn(...) log_printf_safe_context(LOG_LEVEL_WARN, ##__VA_ARGS__)
#define log_info(...) log_printf_safe_context(LOG_LEVEL_INFO, ##__VA_ARGS__)
#define log_debug(...) log_printf_safe_context(LOG_LEVEL_DEBUG, ##__VA_ARGS__)
#define log_trace(...) log_printf_safe_context(LOG_LEVEL_TRACE, ##__VA_ARGS__)

// for manually specifying subsystem
#define log_fatal_ex(subsys, fmt, ...) log_printf_safe(LOG_LEVEL_FATAL, subsys, fmt, ##__VA_ARGS__)
#define log_error_ex(subsys, fmt, ...) log_printf_safe(LOG_LEVEL_ERROR, subsys, fmt, ##__VA_ARGS__)
#define log_info_ex(subsys, fmt, ...) log_printf_safe(LOG_LEVEL_INFO, subsys, fmt, ##__VA_ARGS__)

int log_printf(log_level_t level, log_subsys_t subsys, const char* fmt, ...);
int log_vprintf(log_level_t level, log_subsys_t subsys, const char* fmt, va_list ap);

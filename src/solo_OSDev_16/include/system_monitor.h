#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include <libc/stdint.h>

// System monitor screens
void system_monitor_overview_screen();
void system_monitor_memory_screen();

// Main function for System Monitor
void system_monitor_screen();

// Handles navigation for the System monitor, called by IRQ1 for proper interrupt functionality
void system_monitor_handle_input(uint8_t scancode);

void system_monitor_update();

#endif // SYSTEM_MONITOR_H
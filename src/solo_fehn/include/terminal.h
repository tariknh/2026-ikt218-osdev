/*
 * terminal.h - VGA text-mode terminal API
 *
 * The PC firmware initialises a 80x25 character text-mode framebuffer at
 * physical address 0xB8000. Each cell is two bytes: low byte = ASCII
 * character, high byte = attribute (foreground & background colour).
 * Writing to that memory makes characters appear on screen immediately.
 *
 * This module hides the framebuffer behind a tiny "terminal" abstraction
 * so the rest of the kernel can just call printf() and friends.
 */

#pragma once
#include <libc/stdint.h>
#include <libc/stddef.h>

void terminal_initialize(void);
void terminal_putchar(char c);
void terminal_write(const char* str, size_t len);
void terminal_writestring(const char* str);

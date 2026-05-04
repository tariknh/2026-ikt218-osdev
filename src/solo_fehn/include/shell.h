/*
 * shell.h - Tiny line-buffered shell
 *
 * The keyboard handler hands us one character at a time via shell_on_char.
 * We echo the character to the screen and accumulate it in a buffer.  When
 * the user presses Enter, we look up the buffered text in a small command
 * table and run the matching function.
 *
 * Built-in commands:
 *   help     show the list of commands
 *   time     show wall-clock time (HH:MM:SS) from the CMOS RTC
 *   date     show wall-clock date (DD-MM-YYYY) from the CMOS RTC
 *   uptime   show seconds since boot from the PIT tick counter
 *   mem      print kernel heap layout
 *   song     play "Twinkle, Twinkle, Little Star" through the PC speaker
 *   clear    clear the screen
 */

#pragma once

void shell_init(void);
void shell_on_char(char c);

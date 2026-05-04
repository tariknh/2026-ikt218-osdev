#pragma once

// VGA foreground colour constants (background always black = 0x00).
// Format: bits 3:0 foreground, bits 7:4 background.
// Black background (0x0X) is assumed throughout the game.
#define COLOR_BORDER 0x0E // yellow
#define COLOR_STATUS_TEXT 0x0F // bright white
#define COLOR_SCORE_VALUE 0x0B // light cyan
#define COLOR_SNAKE_HEAD 0x0A // bright green
#define COLOR_SNAKE_BODY 0x02 // dark green
#define COLOR_FOOD 0x0C // bright red
#define COLOR_BLANK 0x00 // black on black
#define COLOR_GAMEOVER 0x0C // bright red

// Starts the snake game. Returns when the player quits (Q key).
// Assumes gdt_init, terminal_init, idt_init, pic_init, keyboard_init,
// and init_pit have already been called and interrupts are enabled.
void snake_run(void);

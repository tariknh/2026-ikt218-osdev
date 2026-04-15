#pragma once

#include "stdint.h"

#include "../command/command.h"
#include "../keyboard/keyboard.h"
#include "../printing/printing.h"
#include "../string/string.h"
#include "../vga_text_mode_interface/vga_text_mode_interface.h"
#include "../user/user.h"


#define SHELL_MAX_INPUT_LENGTH (VGA_TERMINAL_WIDTH - 4)

typedef struct {
    char current_line[SHELL_MAX_INPUT_LENGTH + 1];
    char submitted_line[SHELL_MAX_INPUT_LENGTH + 1];
    uint16_t current_length;
    uint8_t next_prompt_failed;
} ShellState;

void shell_init(const char* username);
// void shell_set_user(const char* username);
void shell_process_input(void);
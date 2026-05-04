#include "typing_game.h"
#include "terminal.h"
#include "keyboard.h"
#include "pit.h"
#include "memory.h"
#include "libc/stdint.h"
#include "libc/stddef.h"

#define PROMPT_COUNT 5

static const char* prompts[PROMPT_COUNT] = {
    "a poor walker, it often falls down. however, its strong pride makes it puff up its chest without a care.",
    "the extension and contraction of its muscles generates electricity. it glows when in trouble.",
    "its transformation ability is perfect. however, if made to laugh, it cant maintain its disguise.",
    "to steal the life of its target, it slips into the preys shadow and silently waits for an opportunity.",
    "when its huge eyes waver, it sings a mysteriously soothing melody that lulls its enemies to sleep."
};

static uint32_t string_length(const char* str) {
    uint32_t len = 0;

    while (str[len] != '\0') {
        len++;
    }

    return len;
}

static void print_uint(uint32_t n) {
    char buffer[12];
    int i = 10;

    buffer[11] = '\0';

    if (n == 0) {
        terminal_put_char('0');
        return;
    }

    while (n > 0 && i >= 0) {
        buffer[i--] = '0' + (n % 10);
        n /= 10;
    }

    terminal_print_string(&buffer[i + 1]);
}

static void print_time_ms(uint32_t ms) {
    uint32_t seconds = ms / 1000;
    uint32_t milliseconds = ms % 1000;

    print_uint(seconds);
    terminal_put_char('.');

    if (milliseconds < 100) {
        terminal_put_char('0');
    }

    if (milliseconds < 10) {
        terminal_put_char('0');
    }

    print_uint(milliseconds);
    terminal_print_string(" seconds");
}

static const char* choose_prompt(void) {
    uint32_t index = get_current_tick() % PROMPT_COUNT;
    return prompts[index];
}

static void show_start_screen(void) {
    terminal_initialize();

    terminal_print_string("=== Typing Game ===\n\n");
    terminal_print_string("A random sentence will appear.\n");
    terminal_print_string("Type as fast and accurate as you can.\n\n");
    terminal_print_string("Rules:\n");
    terminal_print_string("1. Timer starts on first typed character.\n");
    terminal_print_string("2. No backspace.\n");
    terminal_print_string("3. Typos are counted but game will continue.\n\n");
    terminal_print_string("Press ENTER to start.\n");
    terminal_print_string("Press M to return to the menu.\n");
}

static void show_results(
    const char* prompt,
    char* typed,
    uint32_t prompt_length,
    uint32_t mistakes,
    uint32_t elapsed_ms
) {
    uint32_t correct_chars = prompt_length - mistakes;
    uint32_t accuracy = 0;
    uint32_t wpm = 0;

    if (prompt_length > 0) {
        accuracy = (correct_chars * 100) / prompt_length;
    }

    if (elapsed_ms > 0) {
        // Standard typing-test estimate:
        // WPM = (correct_chars / 5) / elapsed_minutes
        // WPM = correct_chars * 12000 / elapsed_ms
        wpm = (correct_chars * 12000) / elapsed_ms;
    }

    terminal_initialize();

    terminal_print_string("=== Game Results ===\n\n");

    terminal_print_string("Prompt:\n");
    terminal_print_string(prompt);
    terminal_print_string("\n\n");

    terminal_print_string("Your input:\n");
    terminal_print_string(typed);
    terminal_print_string("\n\n");

    terminal_print_string("Time: ");
    print_time_ms(elapsed_ms);
    terminal_print_string("\n");

    terminal_print_string("Correct characters: ");
    print_uint(correct_chars);
    terminal_print_string(" / ");
    print_uint(prompt_length);
    terminal_print_string("\n");

    terminal_print_string("Mistakes: ");
    print_uint(mistakes);
    terminal_print_string("\n");

    terminal_print_string("Accuracy: ");
    print_uint(accuracy);
    terminal_print_string("%\n");

    terminal_print_string("WPM: ");
    print_uint(wpm);
    terminal_print_string("\n\n");

    terminal_print_string("Press R to restart.\n");
    terminal_print_string("Press M to return to menu.\n");
}

void typing_game_run(void) {
    keyboard_set_echo(0);

    while (1) {
        show_start_screen();
        keyboard_clear_buffer();

        char start_key = 0;

        while (1) {
            start_key = keyboard_wait_key();

            if (start_key == '\n') {
                break;
            }

            if (start_key == 'm') {
                return;
            }
        }

        const char* prompt = choose_prompt();
        uint32_t prompt_length = string_length(prompt);

        char* typed = (char*)malloc(prompt_length + 1);

        if (!typed) {
            terminal_initialize();
            terminal_print_string("Failed to allocate typing buffer.\n");
            terminal_print_string("Press any key to return to menu.\n");
            keyboard_wait_key();
            return;
        }

        for (uint32_t i = 0; i <= prompt_length; i++) {
            typed[i] = '\0';
        }

        terminal_initialize();
        terminal_print_string("=== Typing Speed Game ===\n\n");
        terminal_print_string("Type this:\n\n");
        terminal_print_string(prompt);
        terminal_print_string("\n\n");
        terminal_print_string("Input:\n> ");

        uint32_t mistakes = 0;
        uint32_t position = 0;
        uint32_t start_tick = 0;
        uint32_t end_tick = 0;

        keyboard_clear_buffer();

        while (position < prompt_length) {
            char c = keyboard_wait_key();

            // Ignore Enter, Tab, and Backspace during the typing part
            if (c == '\n' || c == '\t' || c == '\b') {
                continue;
            }

            if (position == 0) {
                start_tick = get_current_tick();
            }

            typed[position] = c;
            terminal_put_char(c);

            if (c != prompt[position]) {
                mistakes++;
            }

            position++;
        }

        end_tick = get_current_tick();
        typed[prompt_length] = '\0';

        uint32_t elapsed_ms = end_tick - start_tick;

        show_results(prompt, typed, prompt_length, mistakes, elapsed_ms);

        free(typed);

        keyboard_clear_buffer();

        while (1) {
            char next = keyboard_wait_key();

            if (next == 'r') {
                break;
            }

            if (next == 'm') {
                return;
            }
        }
    }
}
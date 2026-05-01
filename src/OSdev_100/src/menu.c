#include "../include/menu.h"

#include "../include/libc/stdint.h"
#include "../include/libc/stdio.h"
#include "../include/memory.h"
#include "../include/matrix_rain.h"
#include "../include/monitor.h"
#include "../include/keyboard.h"
#include "../include/pit.h"
#include "../include/song.h"
#include "../include/system_checks.h"
#include "../include/text_editor.h"

static volatile char* const menu_vga = (volatile char*)0xB8000;
static uint32_t menu_star_seed = 0x13572468u;

static uint32_t menu_star_random(void) {
    menu_star_seed = (menu_star_seed * 1664525u) + 1013904223u;
    return menu_star_seed;
}

static void menu_put_char(int row, int column, char c, char color) {
    int offset;

    if (row < 0 || row >= 25 || column < 0 || column >= 80) {
        return;
    }

    offset = (row * 80 + column) * 2;
    menu_vga[offset] = c;
    menu_vga[offset + 1] = color;
}

static void menu_write_text(int row, int column, const char* text, char color) {
    int i = 0;
    uint8_t old_color;

    monitor_set_cursor((size_t)row, (size_t)column);
    old_color = monitor_get_color();
    monitor_set_color((uint8_t)color);
    while (text[i] != '\0' && (column + i) < 80) {
        monitor_put(text[i]);
        i++;
    }
    monitor_set_color(old_color);
}

static void menu_write_centered(int row, const char* text, char color) {
    int length = 0;
    int start_column;

    while (text[length] != '\0') {
        length++;
    }

    start_column = (80 - length) / 2;
    if (start_column < 0) {
        start_column = 0;
    }

    menu_write_text(row, start_column, text, color);
}

static void menu_draw_side_stars_frame(void) {
    static int initialized = 0;
    static const int star_rows[16] = {
        1, 3, 6, 9, 12, 15, 18, 21,
        2, 4, 7, 10, 13, 16, 19, 22
    };
    static const int star_columns[16] = {
        4, 9, 6, 11, 8, 5, 5, 10,
        62, 67, 64, 69, 66, 71, 63, 68
    };
    static char star_chars[16];
    static int next_left = 0;
    static int next_right = 1;
    int i;

    if (!initialized) {
        for (i = 0; i < 16; i++) {
            star_chars[i] = ((i % 5) == 0) ? '*' : '.';
            menu_put_char(star_rows[i], star_columns[i], star_chars[i], COLOR_DARK_GRAY);
        }
        initialized = 1;
    }

    for (i = 0; i < 2; i++) {
        int id;

        if (i == 0) {
            id = next_left;
            next_left += 2;
            if (next_left >= 16) {
                next_left = 0;
            }
        } else {
            id = next_right;
            next_right += 2;
            if (next_right >= 16) {
                next_right = 1;
            }
        }

        menu_put_char(star_rows[id], star_columns[id], ' ', COLOR_BLACK);

        if (star_chars[id] == '.') {
            star_chars[id] = '*';
            menu_put_char(star_rows[id], star_columns[id], star_chars[id], COLOR_LIGHT_GRAY);
        } else {
            star_chars[id] = '.';
            menu_put_char(star_rows[id], star_columns[id], star_chars[id], COLOR_DARK_GRAY);
        }

        if ((menu_star_random() & 15u) == 0u) {
            star_chars[id] = '.';
            menu_put_char(star_rows[id], star_columns[id], star_chars[id], COLOR_DARK_GRAY);
        }
    }
}

static void show_boot_screen(void) {
    key_action_t action = KEY_ACTION_NONE;
    const char* lines[] = {
        " _   _      _ _        __        __         _     _ _ ",
        "| | | | ___| | | ___   \\ \\      / /__  _ __| | __| | |",
        "| |_| |/ _ \\ | |/ _ \\   \\ \\ /\\ / / _ \\| '__| |/ _` | |",
        "|  _  |  __/ | | (_) |   \\ V  V / (_) | |  | | (_| |_|",
        "|_| |_|\\___|_|_|\\___/     \\_/\\_/ \\___/|_|  |_|\\__,_(_)",
        "",
        "IKT218-G Advanced Operating Systems",
        "OSdev_100",
        "",
        "Press Enter to continue"
    };
    const size_t line_count = sizeof(lines) / sizeof(lines[0]);
    const size_t screen_width = 80;
    const size_t screen_height = 25;
    const size_t logo_width = 57;
    const size_t start_row = (screen_height - line_count) / 2;
    static const int boot_star_rows[12] = {
        2, 4, 6, 8, 10, 12,
        3, 5, 7, 9, 11, 13
    };
    static const int boot_star_columns[12] = {
        6, 12, 8, 15, 10, 14,
        63, 69, 66, 72, 64, 70
    };
    static char boot_star_chars[12];
    int next_star = 0;
    int previous_star = -1;

    monitor_initialize();
    monitor_hide_cursor();
    keyboard_set_mode(KEYBOARD_MODE_COMMAND);

    for (size_t i = 0; i < line_count; i++) {
        size_t line_length = 0;
        size_t start_column;

        while (lines[i][line_length] != '\0') {
            line_length++;
        }

        if (i < 5) {
            start_column = (screen_width - logo_width) / 2;
        } else {
            start_column = (screen_width - line_length) / 2;
        }

        monitor_set_cursor(start_row + i, start_column);
        if (i == 7) {
            monitor_set_color(COLOR_LIGHT_GREEN);
        } else if (i == 9) {
            monitor_set_color(COLOR_YELLOW);
        }
        printf("%s", lines[i]);
        monitor_set_color(COLOR_WHITE);
    }

    for (int i = 0; i < 12; i++) {
        boot_star_chars[i] = ((i % 4) == 0) ? '*' : '.';
        menu_put_char(boot_star_rows[i], boot_star_columns[i], boot_star_chars[i], COLOR_DARK_GRAY);
    }

    while (action != KEY_ACTION_ENTER) {
        action = keyboard_take_action();
        if (action == KEY_ACTION_ENTER) {
            break;
        }

        if (previous_star >= 0) {
            boot_star_chars[previous_star] = '.';
            menu_put_char(boot_star_rows[previous_star], boot_star_columns[previous_star], '.', COLOR_DARK_GRAY);
        }

        boot_star_chars[next_star] = '*';
        menu_put_char(boot_star_rows[next_star], boot_star_columns[next_star], '*', COLOR_LIGHT_GRAY);
        previous_star = next_star;
        next_star = (next_star + 1) % 12;
        sleep_interrupt(220);
    }
}

static void show_main_menu(void) {
    const int left_column = 8;
    static const char* earth_art[] = {
        "                    .   ",
        "                   .'.",
        "                   |o|",
        "                  .'o'.",
        "                  |.-.|",
        "                  '   '",
        "                   ( )",
        "                    )",
        "                   ( )",
        "",
        "               ____",
        "          .-'\"\"p 8o\"\"`-.",
        "       .-'8888P'Y.`Y[ ' `-.",
        "     ,']88888b.J8oo_      '`."
    };
    int earth_index;

    monitor_initialize();
    monitor_hide_cursor();
    keyboard_set_mode(KEYBOARD_MODE_COMMAND);
    menu_write_text(7, left_column, "UiAOS Assignment Menu", COLOR_YELLOW);
    menu_write_text(8, left_column, "---------------------", COLOR_DARK_GRAY);
    menu_write_text(6, 58, "  .-.", COLOR_LIGHT_GRAY);
    menu_write_text(7, 58, " ( (", COLOR_LIGHT_GRAY);
    menu_write_text(8, 58, "  `-'", COLOR_LIGHT_GRAY);
    for (earth_index = 0; earth_index < 14; earth_index++) {
        menu_write_text(11 + earth_index, 34, earth_art[earth_index], COLOR_LIGHT_GRAY);
    }
    menu_write_text(11, left_column, "[1] System Checks", COLOR_LIGHT_GRAY);
    menu_write_text(13, left_column, "[2] Assignment 5 Music Player", COLOR_LIGHT_GRAY);
    menu_write_text(15, left_column, "[3] Text Editor", COLOR_LIGHT_GRAY);
    menu_write_text(17, left_column, "[4] Matrix Rain", COLOR_LIGHT_GRAY);
    menu_write_text(20, left_column, "[ESC] Return from a demo", COLOR_DARK_GRAY);
}

static void run_assignment5_demo(void) {
    Song songs[] = {
        {music_1, sizeof(music_1) / sizeof(Note)},
        {starwars_theme, sizeof(starwars_theme) / sizeof(Note)},
        {battlefield_1942_theme, sizeof(battlefield_1942_theme) / sizeof(Note)}
    };
    const char* titles[] = {
        "Mario Theme",
        "Star Wars Theme",
        "Battlefield 1942"
    };
    const int option_rows[] = {10, 12, 14};

    int ch;
    int selected_index = -1;
    int play_result;

    monitor_initialize();
    monitor_hide_cursor();
    keyboard_set_mode(KEYBOARD_MODE_COMMAND);

    menu_write_text(6, 8, "Music Player", COLOR_YELLOW);
    menu_write_text(7, 8, "------------", COLOR_DARK_GRAY);
    menu_write_text(10, 8, "[1] Mario Theme", COLOR_LIGHT_GRAY);
    menu_write_text(12, 8, "[2] Star Wars Theme", COLOR_LIGHT_GRAY);
    menu_write_text(14, 8, "[3] Battlefield 1942", COLOR_LIGHT_GRAY);
    menu_write_text(17, 8, "[ESC] Return to menu", COLOR_DARK_GRAY);

    menu_write_text(6, 42, "Music Output", COLOR_YELLOW);
    menu_write_text(7, 42, "------------", COLOR_DARK_GRAY);
    menu_write_text(9, 42, "Select a song on the left.", COLOR_DARK_GRAY);

    music_player_set_log_area(9, 42);

    while (1) {
        if (keyboard_take_action() == KEY_ACTION_ESCAPE) {
            return;
        }

        ch = keyboard_take_char();
        if (ch == -1) {
            sleep_interrupt(60);
            continue;
        }

        if (ch >= '1' && ch <= '3') {
            selected_index = ch - '1';
            menu_write_text(option_rows[0], 8, "[1] Mario Theme", selected_index == 0 ? COLOR_WHITE : COLOR_LIGHT_GRAY);
            menu_write_text(option_rows[1], 8, "[2] Star Wars Theme", selected_index == 1 ? COLOR_WHITE : COLOR_LIGHT_GRAY);
            menu_write_text(option_rows[2], 8, "[3] Battlefield 1942", selected_index == 2 ? COLOR_WHITE : COLOR_LIGHT_GRAY);

            while (1) {
                play_result = play_music_song(titles[selected_index], &songs[selected_index]);
                if (play_result == MUSIC_RESULT_EXIT_MENU) {
                    return;
                }
                if (play_result >= MUSIC_RESULT_SWITCH_1 && play_result <= MUSIC_RESULT_SWITCH_3) {
                    selected_index = play_result - 1;
                    menu_write_text(option_rows[0], 8, "[1] Mario Theme", selected_index == 0 ? COLOR_WHITE : COLOR_LIGHT_GRAY);
                    menu_write_text(option_rows[1], 8, "[2] Star Wars Theme", selected_index == 1 ? COLOR_WHITE : COLOR_LIGHT_GRAY);
                    menu_write_text(option_rows[2], 8, "[3] Battlefield 1942", selected_index == 2 ? COLOR_WHITE : COLOR_LIGHT_GRAY);
                    continue;
                }
                break;
            }
        }
    }
}

static void run_matrix_rain_demo(void) {
    run_matrix_rain();
}

void run_menu_loop(void) {
    show_boot_screen();

    while (1) {
        int ch;
        show_main_menu();
        while (1) {
            if (keyboard_take_action() == KEY_ACTION_ESCAPE) {
                show_boot_screen();
                show_main_menu();
                continue;
            }

            ch = keyboard_take_char();
            if (ch == -1) {
                menu_draw_side_stars_frame();
                sleep_interrupt(320);
                continue;
            }

            if (ch == '1') {
                run_system_checks();
                break;
            } else if (ch == '2') {
                run_assignment5_demo();
                break;
            } else if (ch == '3') {
                run_text_editor();
                break;
            } else if (ch == '4') {
                run_matrix_rain_demo();
                break;
            }
        }
    }
}

#include "../include/text_editor.h"

#include "../include/keyboard.h"
#include "../include/monitor.h"

static void text_editor_write_text(int row, int column, const char* text, char color) {
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

void run_text_editor(void) {
    const int left_column = 8;
    int i;

    monitor_initialize();
    monitor_show_cursor();
    keyboard_set_mode(KEYBOARD_MODE_COMMAND);

    text_editor_write_text(4, left_column, "Text Editor", COLOR_YELLOW);
    text_editor_write_text(5, left_column, "-----------", COLOR_DARK_GRAY);
    text_editor_write_text(7, left_column, "Write freely below. Press ESC to return.", COLOR_LIGHT_GRAY);
    text_editor_write_text(8, left_column, "Your text stays inside the editor area.", COLOR_DARK_GRAY);

    monitor_set_color(COLOR_DARK_GRAY);
    monitor_set_cursor(10, left_column);
    for (i = 0; i < 64; i++) {
        monitor_put('-');
    }
    monitor_set_color(COLOR_WHITE);

    keyboard_enter_text_mode(12, left_column, 64, 12);

    while (1) {
        if (keyboard_take_action() == KEY_ACTION_ESCAPE) {
            keyboard_set_mode(KEYBOARD_MODE_COMMAND);
            return;
        }
    }
}

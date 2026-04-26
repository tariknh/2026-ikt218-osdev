#include "feature_menu.h"
#include "typing_game.h"
#include "terminal.h"
#include "keyboard.h"
#include "song.h"
#include "pit.h"

static void play_music_once(void) {
    terminal_initialize();

    terminal_print_string("=== Music Player ===\n\n");
    terminal_print_string("Playing song...\n\n");

    Song song = {
        music_1,
        sizeof(music_1) / sizeof(Note)
    };

    SongPlayer* player = create_song_player();

    if (!player) {
        terminal_print_string("Failed to create song player.\n");
        terminal_print_string("Press any key to return to menu.\n");
        keyboard_wait_key();
        return;
    }

    player->play_song(&song);

    terminal_print_string("\nSong finished.\n");
    terminal_print_string("Press any key to return to menu.\n");

    keyboard_clear_buffer();
    keyboard_wait_key();
}

static void draw_menu(void) {
    terminal_initialize();

    terminal_print_string("=== UIAOS Feature Menu ===\n\n");
    terminal_print_string("1. Play music player\n");
    terminal_print_string("2. Play typing speed game\n\n");
    terminal_print_string("Choose an option: ");
}

void feature_menu_run(void) {
    keyboard_set_echo(0);

    while (1) {
        draw_menu();
        keyboard_clear_buffer();

        char choice = keyboard_wait_key();

        if (choice == '1') {
            play_music_once();
        } else if (choice == '2') {
            typing_game_run();
        } else {
            terminal_print_string("\nInvalid choice.\n");
            terminal_print_string("Press any key to try again.\n");
            keyboard_wait_key();
        }
    }
}
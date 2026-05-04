#include "kernel/apps.h"

#include "arch/i386/isr.h"
#include "arch/i386/keyboard.h"
#include "kernel/pit.h"
#include "stdbool.h"
#include "stdio.h"
#include "song/song.h"

bool keyboard_escape_pressed(void) {
    char key = 0;

    return keyboard_try_read(&key) && key == KEYBOARD_SCANCODE_TO_CHAR[KEY_SCANCODE_ESCAPE];
}

static bool play_song_library(void) {
    Song songs[] = {
        SONG_FROM_NOTES(music_1),
        SONG_FROM_NOTES(starwars_theme),
        SONG_FROM_NOTES(battlefield_1942_theme),
        SONG_FROM_NOTES(music_2),
        SONG_FROM_NOTES(music_3),
        SONG_FROM_NOTES(music_4),
        SONG_FROM_NOTES(music_5),
        SONG_FROM_NOTES(music_6),
    };

    uint32_t song_count = sizeof(songs) / sizeof(songs[0]);

    for (uint32_t i = 0; i < song_count; i++) {
        if (keyboard_escape_pressed()) {
            return false;
        }

        printf("Playing song %d/%d\n", (int) (i + 1), (int) song_count);
        if (!play_song_until(&songs[i], keyboard_escape_pressed)) {
            return false;
        }
        sleep_interrupt(500);
    }

    return true;
}

void run_music_player(void) {
    terminal_clear();
    printf("=== Music Player ===       ESC: menu\n\n");
    if (play_song_library()) {
        printf("\nSong player finished\n\n");
        printf("Press any key to return to the menu...");
        keyboard_wait_read();
    } else {
        terminal_clear();
    }
}

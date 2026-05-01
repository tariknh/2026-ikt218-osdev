#include "../include/song.h"
#include "../include/io.h"
#include "../include/keyboard.h"
#include "../include/pit.h"
#include "../include/memory.h"
#include "../include/monitor.h"
#include "../include/libc/stdio.h"

/* 0x61 is the PC speaker control port - defined in pit.h as PC_SPEAKER_PORT
 * bit 0 and bit 1 are the ones we care about for turning sound on/off
 */

// -------------------------- //
// keyboard / control helpers
// -------------------------- //

static int music_exit_requested(void) {
    return keyboard_take_action() == KEY_ACTION_ESCAPE;
}

static int music_poll_result(void) {
    int ch;

    if (music_exit_requested()) {
        return MUSIC_RESULT_EXIT_MENU;
    }

    ch = keyboard_take_char();
    if (ch >= '1' && ch <= '3') {
        return ch - '0';
    }

    return MUSIC_RESULT_COMPLETED;
}

static uint32_t music_log_row = 9;
static uint32_t music_log_column = 42;
static uint32_t music_log_height = 12;
static uint32_t music_log_cursor = 0;

// -------------------------- //
// music log / screen helpers
// -------------------------- //

// clear one line in the little music log area on the right side
static void music_clear_line(uint32_t row) {
    uint32_t i;

    monitor_set_cursor(row, music_log_column);
    for (i = 0; i < 34; i++) {
        monitor_put(' ');
    }
}

// wipe the whole music log so a new song can start clean
static void music_clear_log_area(void) {
    uint32_t i;

    for (i = 0; i < music_log_height; i++) {
        music_clear_line(music_log_row + i);
    }
    music_log_cursor = 0;
}

// write one log line for the music player and keep the old screen colors safe
static void music_write_log_line(const char* text) {
    uint8_t old_color;

    if (music_log_cursor >= music_log_height) {
        music_clear_log_area();
    }

    music_clear_line(music_log_row + music_log_cursor);
    monitor_set_cursor(music_log_row + music_log_cursor, music_log_column);
    old_color = monitor_get_color();
    monitor_set_color(COLOR_DARK_GRAY);
    printf("%s", text);
    monitor_set_color(old_color);
    music_log_cursor++;
}

// tiny helper to append plain text into the note log buffer
static void music_append_text(char* buffer, int* pos, const char* text) {
    while (*text != '\0' && *pos < 34) {
        buffer[*pos] = *text;
        (*pos)++;
        text++;
    }
}

// same idea as above, but for numbers like note index, frequency and duration
static void music_append_uint(char* buffer, int* pos, uint32_t value) {
    char digits[10];
    int count = 0;

    if (value == 0) {
        if (*pos < 34) {
            buffer[*pos] = '0';
            (*pos)++;
        }
        return;
    }

    while (value > 0 && count < 10) {
        digits[count++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (count > 0 && *pos < 34) {
        buffer[*pos] = digits[--count];
        (*pos)++;
    }
}

// build one readable line like "Note 3: F=440Hz T=250ms"
static void music_append_note_line(char* buffer, uint32_t index, uint32_t frequency, uint32_t duration) {
    int pos = 0;

    music_append_text(buffer, &pos, "Note ");
    music_append_uint(buffer, &pos, index);
    music_append_text(buffer, &pos, ": F=");
    music_append_uint(buffer, &pos, frequency);
    music_append_text(buffer, &pos, "Hz T=");
    music_append_uint(buffer, &pos, duration);
    music_append_text(buffer, &pos, "ms");
    buffer[pos] = '\0';
}

// move the little music log if we want it somewhere else on the screen
void music_player_set_log_area(uint32_t row, uint32_t column) {
    music_log_row = row;
    music_log_column = column;
    music_clear_log_area();
}

// -------------------------- //
// timing helper while music is playing
// -------------------------- //

// sleep in small chunks so the song can still react to esc or song switch keys
static int music_sleep_interruptible(uint32_t milliseconds) {
    while (milliseconds > 0) {
        uint32_t slice = milliseconds > 10 ? 10 : milliseconds;
        int poll_result;

        poll_result = music_poll_result();
        if (poll_result != MUSIC_RESULT_COMPLETED) {
            return poll_result;
        }

        sleep_interrupt(slice);
        milliseconds -= slice;

        poll_result = music_poll_result();
        if (poll_result != MUSIC_RESULT_COMPLETED) {
            return poll_result;
        }
    }

    return MUSIC_RESULT_COMPLETED;
}

// -------------------------- //
// actual speaker / sound control
// -------------------------- //

void enable_speaker() {
    uint8_t temp = inb(PC_SPEAKER_PORT);
    if (temp != (temp | 3)) {
        outb(PC_SPEAKER_PORT, temp | 3);
    }
}

void disable_speaker() {
    uint8_t temp = inb(PC_SPEAKER_PORT) & 0xFC;
    outb(PC_SPEAKER_PORT, temp);
}

// set PIT channel 2 to the note frequency we want and then let the speaker use it
void play_sound(uint32_t frequency) {
    if (frequency == 0) {
        disable_speaker();
        return;
    }

    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;

    outb(PIT_CMD_PORT, 0xB6);

    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    enable_speaker();
}

void stop_sound() {
    uint8_t temp = inb(PC_SPEAKER_PORT) & 0xFC;
    outb(PC_SPEAKER_PORT, temp);
}

// -------------------------- //
// actual song playback logic
// -------------------------- //

// main song loop: go note by note, play it for its time, then move to the next one
int play_song_impl(Song *song) {
    if (song == 0 || song->notes == 0) {
        return MUSIC_RESULT_COMPLETED;
    }

    enable_speaker();

    for (uint32_t i = 0; i < song->length; i++) {
        Note n;
        int poll_result;

        poll_result = music_poll_result();
        if (poll_result != MUSIC_RESULT_COMPLETED) {
            disable_speaker();
            return poll_result;
        }

        n = song->notes[i];

        {
            char note_line[35];
            music_append_note_line(note_line, i, n.frequency, n.duration);
            music_write_log_line(note_line);
        }

        if (n.frequency == R) {
            stop_sound();
        } else {
            play_sound(n.frequency);
        }

        poll_result = music_sleep_interruptible(n.duration);
        if (poll_result != MUSIC_RESULT_COMPLETED) {
            disable_speaker();
            return poll_result;
        }

        stop_sound();
        poll_result = music_sleep_interruptible(20);
        if (poll_result != MUSIC_RESULT_COMPLETED) {
            disable_speaker();
            return poll_result;
        }
    }

    disable_speaker();
    return MUSIC_RESULT_COMPLETED;
}

// small wrapper so the rest of the os can call the player more cleanly
SongPlayer* create_song_player() {
    static SongPlayer player_instance;
    SongPlayer* player = &player_instance;
    player->play_song = play_song_impl;
    return player;
}

// direct wrapper if we just want to play one song
int play_song(Song *song) {
    return play_song_impl(song);
}

// simple path that just plays the default song list
int play_music(void) {
    Song songs[] = {
        {music_1, sizeof(music_1) / sizeof(Note)}
    };
    uint32_t n_songs = sizeof(songs) / sizeof(Song);
    SongPlayer* player = create_song_player();

    if (player == NULL) {
        return MUSIC_RESULT_EXIT_MENU;
    }

    for (uint32_t i = 0; i < n_songs; i++) {
        if (music_exit_requested()) {
            disable_speaker();
            return MUSIC_RESULT_EXIT_MENU;
        }
        printf("Playing Song...\n");
        if (player->play_song(&songs[i]) != MUSIC_RESULT_COMPLETED) {
            disable_speaker();
            return MUSIC_RESULT_EXIT_MENU;
        }
        printf("Finished playing the song.\n");
    }

    return MUSIC_RESULT_COMPLETED;
}

// nicer menu path: clear log, show title, then play the chosen song
int play_music_song(const char* title, Song* song) {
    SongPlayer* player = create_song_player();
    int play_result;

    if (player == NULL) {
        return MUSIC_RESULT_EXIT_MENU;
    }

    music_clear_log_area();
    music_write_log_line("Playing Song...");
    {
        char title_line[35];
        int i = 0;
        const char* prefix = "Track: ";

        while (prefix[i] != '\0' && i < 34) {
            title_line[i] = prefix[i];
            i++;
        }

        {
            int j = 0;
            while (title[j] != '\0' && i < 34) {
                title_line[i++] = title[j++];
            }
        }

        title_line[i] = '\0';
        music_write_log_line(title_line);
    }

    play_result = player->play_song(song);
    if (play_result != MUSIC_RESULT_COMPLETED) {
        disable_speaker();
        return play_result;
    }

    music_write_log_line("Finished playing the song.");

    return MUSIC_RESULT_COMPLETED;
}

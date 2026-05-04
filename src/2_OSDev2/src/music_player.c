#include <libc/stddef.h>
#include <libc/stdint.h>

#include "song/song.h"
#include "io.h"
#include "kernel_memory.h"
#include "pit.h"
#include "terminal.h"

#define PIT_CMD_PORT      0x43
#define PIT_CHANNEL2_PORT 0x42
#define PC_SPEAKER_PORT   0x61
#define PIT_SPEAKER_CMD   0xB6

Song get_demo_song(void) {
    Song song;
    song.notes = starwars_theme;
    song.length = (uint32_t)(sizeof(starwars_theme) / sizeof(starwars_theme[0]));
    return song;
}

void enable_speaker(void) {
    uint8_t speaker_state = inb(PC_SPEAKER_PORT);
    if ((speaker_state & 0x03u) != 0x03u) {
        outb(PC_SPEAKER_PORT, (uint8_t)(speaker_state | 0x03u));
    }
}

void disable_speaker(void) {
    uint8_t speaker_state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, (uint8_t)(speaker_state & ~0x03u));
}

void play_sound(uint32_t frequency) {
    if (frequency == 0u) {
        stop_sound();
        return;
    }

    uint16_t divisor = (uint16_t)(PIT_BASE_FREQUENCY / frequency);

    outb(PIT_CMD_PORT, PIT_SPEAKER_CMD);
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFFu));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFFu));

    enable_speaker();
}

void stop_sound(void) {
    disable_speaker();
}

void play_song_impl(Song* song) {
    if (song == NULL || song->notes == NULL || song->length == 0u) {
        terminal_write("No song data available.\n");
        return;
    }

    terminal_write("Playing song on PC speaker...\n");

    for (uint32_t i = 0; i < song->length; i++) {
        Note* note = &song->notes[i];

        if (note->frequency == R) {
            stop_sound();
        } else {
            play_sound(note->frequency);
        }

        sleep_interrupt(note->duration);
        stop_sound();
        sleep_interrupt(20);
    }

    terminal_write("Finished playing song.\n");
}

SongPlayer* create_song_player(void) {
    SongPlayer* player = (SongPlayer*)malloc(sizeof(SongPlayer));
    if (player == NULL) {
        return NULL;
    }

    player->play_song = play_song_impl;
    return player;
}
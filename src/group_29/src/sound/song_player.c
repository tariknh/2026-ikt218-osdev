#include "song_player.h"
#include "song.h"

#include "libc/stddef.h"
#include "../pit/pit.h"

void enable_speaker(void) {
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, (uint8_t)(state | 0x03U));
}

void disable_speaker(void) {
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, (uint8_t)(state & (uint8_t)(~0x03U)));
}

void play_sound(uint32_t frequency) {
    uint16_t divisor;

    if (frequency == 0U) {
        stop_sound();
        return;
    }

    divisor = (uint16_t)(PIT_BASE_FREQUENCY / frequency);

    outb(PIT_CMD_PORT, 0xB6U);
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFFU));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFFU));
    enable_speaker();
}

void stop_sound(void) {
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, (uint8_t)(state & (uint8_t)(~0x03U)));
}

void play_song_impl(Song *song) {
    uint32_t i;

    if (song == NULL || song->notes == NULL || song->length == 0U) {
        return;
    }

    enable_speaker();

    for (i = 0U; i < song->length; ++i) {
        play_sound(song->notes[i].frequency);
        sleep_interrupt(song->notes[i].duration);
        stop_sound();
    }

    disable_speaker();
}

void play_song(Song *song) {
    play_song_impl(song);
}

SongPlayer* create_song_player(void) {
    static SongPlayer player;

    player.play_song = play_song;
    return &player;
}

uint32_t get_song_count(void) {
    return (uint32_t)(sizeof(built_in_songs) / sizeof(built_in_songs[0]));
}

void play_song_by_index(uint32_t index) {
    SongPlayer* player;

    if (index >= get_song_count()) {
        return;
    }

    player = create_song_player();
    player->play_song(&built_in_songs[index]);
}

void play_default_song(void) {
    play_song_by_index(0U);
}

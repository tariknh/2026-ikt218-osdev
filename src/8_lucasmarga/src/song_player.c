#include "song.h"
#include "pit.h"
#include "io.h"
#include "memory.h"
#include "terminal.h"

static void enable_speaker(void) {
    uint8_t value = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, value | 0x03);
}

static void disable_speaker(void) {
    uint8_t value = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, value & 0xFC);
}

static void play_sound(uint32_t frequency) {
    if (frequency == 0) {
        disable_speaker();
        return;
    }

    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;

    outb(PIT_CMD_PORT, 0xB6);
    outb(PIT_CHANNEL2_PORT, divisor & 0xFF);
    outb(PIT_CHANNEL2_PORT, (divisor >> 8) & 0xFF);

    enable_speaker();
}

static void stop_sound(void) {
    disable_speaker();
}

void play_song_impl(Song* song) {
    if (!song || !song->notes) {
        return;
    }

    for (uint32_t i = 0; i < song->length; i++) {
        play_sound(song->notes[i].frequency);
        sleep_interrupt(song->notes[i].duration);
        stop_sound();
        sleep_interrupt(20); // small gap
    }

    stop_sound();
}

SongPlayer* create_song_player(void) {
    SongPlayer* player = (SongPlayer*)malloc(sizeof(SongPlayer));

    if (!player) {
        terminal_print_string("Failed to create SongPlayer.\n");
        return 0;
    }

    player->play_song = play_song_impl;
    return player;
}
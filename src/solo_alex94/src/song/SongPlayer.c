#include "song/song.h"
#include "kernel/pit.h"
#include "kernel/memory.h"
#include "ports.h"

#define PIT_SPEAKER_COMMAND 0xB6

static void enable_speaker(void) {
    uint8_t value = port_byte_in(PC_SPEAKER_PORT);

    if ((value & 0x03U) != 0x03U) {
        port_byte_out(PC_SPEAKER_PORT, (uint8_t)(value | 0x03U));
    }
}

static void disable_speaker(void) {
    uint8_t value = port_byte_in(PC_SPEAKER_PORT);

    if ((value & 0x03U) != 0U) {
        port_byte_out(PC_SPEAKER_PORT, (uint8_t)(value & 0xFCU));
    }
}

static void play_sound(uint32_t frequency) {
    uint32_t divisor;

    if (frequency == 0U) {
        return;
    }

    divisor = (uint32_t)(PIT_BASE_FREQUENCY / frequency);

    if (divisor < 2U) {
        divisor = 2U;
    }

    if (divisor > 0xFFFEU) {
        divisor = 0xFFFEU;
    }

    divisor &= 0xFFFEU;

    port_byte_out(PIT_CMD_PORT, PIT_SPEAKER_COMMAND);
    port_byte_out(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFFU));
    port_byte_out(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFFU));

    enable_speaker();
}

static void stop_sound(void) {
    disable_speaker();
}

void play_song_impl(Song *song) {
    uint32_t i;

    if (song == 0 || song->notes == 0 || song->length == 0U) {
        return;
    }

    enable_speaker();

    for (i = 0; i < song->length; i++) {
        uint32_t frequency = song->notes[i].frequency;
        uint32_t duration = song->notes[i].duration;

        if (frequency == 0U) {
            stop_sound();
        } else {
            play_sound(frequency);
        }

        sleep_interrupt(duration);
        stop_sound();
    }

    disable_speaker();
}

SongPlayer* create_song_player(void) {
    SongPlayer* player = (SongPlayer*)malloc(sizeof(SongPlayer));

    if (player == 0) {
        return 0;
    }

    player->play_song = play_song_impl;
    return player;
}

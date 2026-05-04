#include "song.h"
#include <pit.h>
#include <io.h>
#include <memory.h>

#define SPEAKER_PORT 0x61
#define PIT_CH2_PORT 0x42
#define PIT_CH2_CMD 0xB6  /* channel 2, lobyte/hibyte, mode 3, binary */

static void enable_speaker(void) {
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state | 0x03);
}

static void disable_speaker(void) {
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state & ~0x03);
}

static void play_sound(uint32_t frequency) {
    if (frequency == 0)
        return;

    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;

    outb(PIT_CMD_PORT, PIT_CH2_CMD);
    outb(PIT_CH2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CH2_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    enable_speaker();
}

static void stop_sound(void) {
    uint8_t state = inb(SPEAKER_PORT);
    outb(SPEAKER_PORT, state & ~0x02);
}

void play_song_impl(Song *song) {
    enable_speaker();

    for (uint32_t i = 0; i < song->length; i++) {
        Note *note = &song->notes[i];
        play_sound(note->frequency);
        sleep_interrupt(note->duration);
        stop_sound();
    }

    disable_speaker();
}

SongPlayer *create_song_player(void) {
    SongPlayer *player = (SongPlayer *)malloc(sizeof(SongPlayer));
    player->play_song = play_song_impl;
    return player;
}

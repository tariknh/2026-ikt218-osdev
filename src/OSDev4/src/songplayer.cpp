#include "song/song.h"

extern "C" {
#include "pit.h"
#include "io.h"
#include "memory.h"
#include "printf.h"
}

void enable_speaker(void)
{
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, state | 0x03);
}

void disable_speaker(void)
{
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, state & ~0x03);
}

void play_sound(uint32_t frequency)
{
    if (frequency == 0) return;

    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;

    outb(PIT_CMD_PORT, 0xB6);
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    enable_speaker();
}

void stop_sound(void)
{
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, state & ~0x02);
}

void play_song_impl(Song* song)
{
    enable_speaker();

    for (uint32_t i = 0; i < song->length; i++) {
        Note* note = &song->notes[i];
        printf("Note: %d Hz, %d ms\n", note->frequency, note->duration);
        play_sound(note->frequency);
        sleep_busy(note->duration);
        stop_sound();
    }

    disable_speaker();
}

void play_song(Song* song)
{
    play_song_impl(song);
}

SongPlayer* create_song_player(void)
{
    SongPlayer* player = (SongPlayer*)malloc(sizeof(SongPlayer));
    player->play_song = play_song_impl;
    return player;
}


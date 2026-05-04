#include <libc/stdint.h>
#include <libc/stddef.h>

#include "song/song.h"
#include "kernel/pit.h"
#include "kernel/memory.h"

#define PC_SPEAKER_PORT 0x61

int printf(const char *format, ...);

static inline uint8_t inb(uint16_t port)
{
    uint8_t result;
    __asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

void enable_speaker(void)
{
    uint8_t speaker_state = inb(PC_SPEAKER_PORT);

    if (speaker_state != (speaker_state | 3))
    {
        outb(PC_SPEAKER_PORT, speaker_state | 3);
    }
}

void disable_speaker(void)
{
    uint8_t speaker_state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, speaker_state & 0xFC);
}

void play_sound(uint32_t frequency)
{
    if (frequency == 0)
    {
        return;
    }

    uint16_t divisor = (uint16_t)(PIT_BASE_FREQUENCY / frequency);

    outb(PIT_CMD_PORT, 0xB6);
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    enable_speaker();
}

void stop_sound(void)
{
    disable_speaker();
}

void play_song_impl(Song *song)
{
    for (uint32_t i = 0; i < song->length; i++)
    {
        Note *note = &song->notes[i];

        play_sound(note->frequency);
        sleep_interrupt(note->duration);
        stop_sound();

        sleep_interrupt(20);
    }
}

static SongPlayer global_player;

SongPlayer *create_song_player(void)
{
    global_player.play_song = play_song_impl;
    return &global_player;
}

void play_music(void)
{
    Song songs[] = {
        {music_1, sizeof(music_1) / sizeof(Note)}};

    uint32_t n_songs = sizeof(songs) / sizeof(Song);
    SongPlayer *player = create_song_player();

    for (uint32_t i = 0; i < n_songs; i++)
    {
        printf("Playing song...\n");
        player->play_song(&songs[i]);
        printf("Finished playing song.\n");
    }
}
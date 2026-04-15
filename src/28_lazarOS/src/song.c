#include "../include/song.h"
#include <frequencies.h>
#include <pit.h>
#include <pic.h>
#include <memory.h>
#include <libc/stdio.h>

static void enable_speaker(void)
{
    uint8_t speaker_state = inb(PC_SPEAKER_PORT);
    if ((speaker_state & 0x03) != 0x03) {
        outb(PC_SPEAKER_PORT, speaker_state | 0x03);
    }
}

static void disable_speaker(void)
{
    uint8_t speaker_state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, speaker_state & (uint8_t)~0x03);
}

static void stop_sound(void)
{
    disable_speaker();
}

static void play_sound(uint32_t frequency)
{
    uint16_t divisor;

    if (frequency == 0) {
        stop_sound();
        return;
    }

    divisor = (uint16_t)(PIT_BASE_FREQUENCY / frequency);

    /* Channel 2, access mode lobyte/hibyte, square wave mode. */
    outb(PIT_CMD_PORT, 0xB6);
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFF));
    enable_speaker();
}

void play_song_impl(Song *song)
{
    uint32_t i;

    if (!song || !song->notes || song->length == 0) {
        return;
    }

    for (i = 0; i < song->length; i++) {
        Note *note = &song->notes[i];
        play_sound(note->frequency);
        sleep_interrupt(note->duration);
        stop_sound();
    }

    disable_speaker();
}

void play_song(Song *song)
{
    play_song_impl(song);
}

SongPlayer* create_song_player(void)
{
    SongPlayer *player = (SongPlayer *)malloc(sizeof(SongPlayer));
    if (!player) {
        return 0;
    }

    player->play_song = play_song_impl;
    return player;
}

void play_music(void)
{
    Song songs[] = {
        {music_1, (uint32_t)(sizeof(music_1) / sizeof(music_1[0]))}
    };
    uint32_t n_songs = (uint32_t)(sizeof(songs) / sizeof(songs[0]));
    SongPlayer *player = create_song_player();
    uint32_t i;

    if (!player) {
        printf("Failed to create song player.\n");
        return;
    }

    while (1) {
        for (i = 0; i < n_songs; i++) {
            printf("Playing Song...\n");
            player->play_song(&songs[i]);
            printf("Finished playing the song.\n");
            sleep_interrupt(300);
        }
    }
}
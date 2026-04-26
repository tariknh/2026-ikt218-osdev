extern "C" {
    #include "song/song.h"
    #include "kernel/pit.h"
    #include "io.h"
    #include "libc/stdio.h"
    #include "kernel/memory.h"
}

static void enable_speaker(void)
{
    uint8_t tmp = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, tmp | 0x03);
}

static void disable_speaker(void)
{
    uint8_t tmp = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, tmp & (uint8_t)~0x03);
}

static void play_sound(uint32_t frequency)
{
    if (frequency == 0)
        return;

    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;

    outb(PIT_CMD_PORT, 0xB6);
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    /* Connect speaker to PIT channel 2 output */
    uint8_t tmp = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, tmp | 0x03);
}

static void stop_sound(void)
{
    uint8_t tmp = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, tmp & (uint8_t)~0x02); /* clear speaker data bit */
}

extern "C" void play_song_impl(Song *song)
{
    enable_speaker();

    for (uint32_t i = 0; i < song->note_count; i++) {
        Note *note = &song->notes[i];
        printf("  note %u: %u Hz, %u ms\n", i, note->frequency, note->duration);

        if (note->frequency == 0)
            stop_sound();
        else
            play_sound(note->frequency);

        sleep_busy(note->duration);
        stop_sound();
    }

    disable_speaker();
}

extern "C" SongPlayer *create_song_player(void)
{
    SongPlayer *player = (SongPlayer *)malloc(sizeof(SongPlayer));
    player->play_song  = play_song_impl;
    return player;
}

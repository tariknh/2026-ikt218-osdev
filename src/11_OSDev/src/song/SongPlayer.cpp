extern "C" {
    #include "song/song.h"
    #include "kernel/pit.h"
    #include "io.h"
    #include "libc/stdio.h"
    #include "kernel/memory.h"
    #include "isr.h"
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

/* Drain non-q keys from the buffer.
   Returns 1 if 'q' was found (left in the buffer for the caller),
   0 otherwise. */
static int seen_quit_key(void)
{
    while (1) {
        char k = kb_peek();
        if (k == 0) return 0;                      /* buffer empty */
        if (k == 'q' || k == 'Q') return 1;        /* leave q in buffer */
        kb_consume();                               /* discard other key */
    }
}

extern "C" void play_song_impl(Song *song)
{
    enable_speaker();

    for (uint32_t i = 0; i < song->note_count; i++) {
        if (seen_quit_key()) { stop_sound(); disable_speaker(); return; }

        Note *note = &song->notes[i];
        printf("  note %u: %u Hz, %u ms\n", i, note->frequency, note->duration);

        if (note->frequency == 0)
            stop_sound();
        else
            play_sound(note->frequency);

        /* Sleep in 50 ms slices so 'q' is detected almost immediately
           rather than waiting for the entire note to end. */
        const uint32_t SLICE_MS = 50;
        uint32_t remaining = note->duration;
        while (remaining > 0) {
            uint32_t chunk = remaining > SLICE_MS ? SLICE_MS : remaining;
            sleep_busy(chunk);
            remaining -= chunk;
            if (seen_quit_key()) { stop_sound(); disable_speaker(); return; }
        }
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

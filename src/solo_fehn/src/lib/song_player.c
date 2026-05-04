/*
 * song_player.c - PC speaker music player
 *
 * The classic IBM-PC speaker is a one-bit speaker driven by PIT channel 2.
 * To play a tone you tell the PIT to oscillate at the desired frequency,
 * then enable two bits in the speaker control register at I/O port 0x61.
 * To stop the tone you clear those bits (the PIT keeps oscillating, but
 * the speaker is no longer connected to it).
 *
 * Port 0x61 bit layout:
 *   bit 0  Timer 2 gate     - allow PIT channel 2 to drive the speaker
 *   bit 1  Speaker data     - actually output sound
 *
 * Both bits must be set for sound to come out.  We set them with a
 * read-modify-write so we don't disturb the higher bits used by the
 * keyboard controller.
 */

#include <song.h>
#include <pit.h>
#include <io.h>
#include <memory.h>
#include <libc/stdint.h>
#include <libc/stdio.h>

void enable_speaker(void) {
    uint8_t state = inb(PC_SPEAKER_PORT);
    /* Only write if the bits are not already set, to avoid speaker clicks. */
    if ((state & 0x03) != 0x03) {
        outb(PC_SPEAKER_PORT, (uint8_t)(state | 0x03));
    }
}

void disable_speaker(void) {
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, (uint8_t)(state & ~0x03));
}

void play_sound(uint32_t frequency) {
    if (frequency == 0) {
        /* A rest: just make sure no sound is currently being emitted. */
        disable_speaker();
        return;
    }

    /* PIT divisor for this frequency. The chip will then toggle its OUT
       line at the requested rate, which the speaker turns into a tone. */
    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;

    /* Command byte 0xB6:
       channel 2 (bits 6-7 = 10), lobyte/hibyte (bits 4-5 = 11),
       mode 3 (square wave, bits 1-3 = 011), binary (bit 0 = 0). */
    outb(PIT_CMD_PORT, 0xB6);
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    enable_speaker();
}

void stop_sound(void) {
    /* Clear bit 1 only; leave the timer-2 gate alone so the next play_sound
       can re-enable the speaker quickly. */
    uint8_t state = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, (uint8_t)(state & ~0x02));
}

void play_song_impl(Song* song) {
    if (!song || !song->notes) return;

    for (size_t i = 0; i < song->note_count; i++) {
        Note* note = &song->notes[i];
        printf("  note %d/%d: %d Hz for %d ms\n",
               (int)(i + 1), (int)song->note_count,
               (int)note->frequency, (int)note->duration);

        play_sound(note->frequency);
        sleep_interrupt(note->duration);
        stop_sound();
    }

    disable_speaker();
}

SongPlayer* create_song_player(void) {
    SongPlayer* player = (SongPlayer*)malloc(sizeof(SongPlayer));
    if (!player) return (SongPlayer*)0;
    player->play_song = play_song_impl;
    return player;
}

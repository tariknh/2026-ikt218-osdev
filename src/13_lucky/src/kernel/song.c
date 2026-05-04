#include "arch/i386/io.h"
#include "song/song.h"

#include "kernel/pit.h"

#define PC_SPEAKER_PORT 0x61
#define PIT_CHANNEL2_PORT 0x42
#define NOTE_GAP_MS 20

static void speaker_on(uint32_t frequency) {
    if (frequency == R) {
        return;
    }

    uint16_t divisor = (uint16_t) (PIT_BASE_FREQUENCY / frequency);

    outb(PIT_CMD_PORT, 0xB6);
    outb(PIT_CHANNEL2_PORT, (uint8_t) (divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t) ((divisor >> 8) & 0xFF));

    uint8_t speaker_state = inb(PC_SPEAKER_PORT);
    if ((speaker_state & 0x03) != 0x03) {
        outb(PC_SPEAKER_PORT, (uint8_t) (speaker_state | 0x03));
    }
}

static void speaker_off(void) {
    outb(PC_SPEAKER_PORT, (uint8_t) (inb(PC_SPEAKER_PORT) & 0xFC));
}

void play_tone(uint32_t frequency, uint32_t duration) {
    if (frequency == R || duration == 0) {
        speaker_off();
        return;
    }

    speaker_on(frequency);
    sleep_interrupt(duration);
    speaker_off();
}

static bool sleep_interruptible(uint32_t milliseconds, bool (*should_stop)(void)) {
    // Breaks notes into smaller chunks so that the playback can quickly stop
    for (uint32_t elapsed = 0; elapsed < milliseconds; elapsed += 10) {
        if (should_stop != 0 && should_stop()) {
            return false;
        }

        uint32_t chunk = milliseconds - elapsed;
        sleep_interrupt(chunk < 10 ? chunk : 10);
    }

    return true;
}

bool play_song_until(Song *song, bool (*should_stop)(void)) {
    if (song == 0 || song->notes == 0) {
        return true;
    }

    // Plays the song until stopped by the caller
    for (uint32_t i = 0; i < song->length; i++) {
        if (should_stop != 0 && should_stop()) {
            speaker_off();
            return false;
        }

        Note note = song->notes[i];

        if (note.frequency == R) {
            speaker_off();
        } else {
            speaker_on(note.frequency);
        }

        if (!sleep_interruptible(note.duration, should_stop)) {
            speaker_off();
            return false;
        }

        speaker_off();

        if (NOTE_GAP_MS < note.duration) {
            if (!sleep_interruptible(NOTE_GAP_MS, should_stop)) {
                return false;
            }
        }
    }

    return true;
}

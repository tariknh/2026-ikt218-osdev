#include "pcspk.h"
#include "pit.h"
#include "terminal.h"
#include <libc/stdbool.h>

#define PIT_BASE_FREQUENCY 1193180u
#define PIT_CMD_PORT       0x43
#define PIT_CHANNEL2_PORT  0x42
#define SPEAKER_CTRL_PORT  0x61

/* Ch2, lobyte/hibyte, mode 3, binary */
#define PIT_CH2_MODE3_CMD  0xB6
#define NOTE_GAP_MS         40u

static const struct song *bg_song = 0;
static uint32_t bg_index = 0;
static uint32_t bg_next_tick = 0;
static bool bg_loop = false;
static bool bg_active = false;
static bool bg_note_on = false;

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void enable_speaker(void) {
    uint8_t state = inb(SPEAKER_CTRL_PORT);
    outb(SPEAKER_CTRL_PORT, (uint8_t)(state | 0x03u));
}

void disable_speaker(void) {
    uint8_t state = inb(SPEAKER_CTRL_PORT);
    outb(SPEAKER_CTRL_PORT, (uint8_t)(state & (uint8_t)~0x03u));
}

void play_sound(uint32_t frequency_hz) {
    uint8_t speaker_state;

    if (frequency_hz == 0u) {
        stop_sound();
        return;
    }

    uint32_t divisor = PIT_BASE_FREQUENCY / frequency_hz;
    if (divisor == 0u) {
        divisor = 1u;
    }
    if (divisor > 0xFFFFu) {
        divisor = 0xFFFFu;
    }

    outb(PIT_CMD_PORT, PIT_CH2_MODE3_CMD);
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFFu));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFFu));

    /* OSDev pattern: set bits 0 and 1 on port 0x61 if not already set. */
    speaker_state = inb(SPEAKER_CTRL_PORT);
    if (speaker_state != (uint8_t)(speaker_state | 0x03u)) {
        outb(SPEAKER_CTRL_PORT, (uint8_t)(speaker_state | 0x03u));
    }
}

void stop_sound(void) {
    /* OSDev nosound(): clear bits 0 and 1. */
    uint8_t state = (uint8_t)(inb(SPEAKER_CTRL_PORT) & 0xFCu);
    outb(SPEAKER_CTRL_PORT, state);
}

void play_song(const struct song *song) {
    if (!song || !song->notes || song->note_count == 0u) {
        return;
    }

    terminal_printf("Playing: %s (%u notes)\n", song->name, song->note_count);
    for (uint32_t i = 0; i < song->note_count; i++) {
        const struct note n = song->notes[i];
        terminal_printf("Note %u: %u Hz, sleep %u ms (+ gap %u ms)\n",
                        (unsigned int)(i + 1u),
                        (unsigned int)n.frequency_hz,
                        (unsigned int)n.duration_ms,
                        (unsigned int)NOTE_GAP_MS);
        play_sound(n.frequency_hz);
        sleep_interrupt(n.duration_ms);
        stop_sound();
        sleep_interrupt(NOTE_GAP_MS);
    }
    disable_speaker();
}

/* Popcorn theme (simplified, C-major friendly). */
static const struct note popcorn_notes[] = {
    { 784, 120 }, { 880, 120 }, { 784, 180 },
    { 659, 120 }, { 784, 120 }, { 659, 180 },

    { 587, 120 }, { 659, 120 }, { 587, 180 },
    { 523, 120 }, { 587, 120 }, { 523, 220 },

    { 784, 120 }, { 880, 120 }, { 784, 180 },
    { 659, 120 }, { 784, 120 }, { 659, 180 },

    { 587, 120 }, { 659, 120 }, { 587, 180 },
    { 523, 120 }, { 494, 120 }, { 440, 240 }
};

static const struct song demo_song = {
    .notes = popcorn_notes,
    .note_count = (uint32_t)(sizeof(popcorn_notes) / sizeof(popcorn_notes[0])),
    .name = "Popcorn Theme (Simplified)"
};

void play_demo_song(void) {
    play_song(&demo_song);
}

void pcspk_bg_start(const struct song *song, int loop) {
    if (!song || !song->notes || song->note_count == 0u) {
        return;
    }

    bg_song = song;
    bg_index = 0;
    bg_next_tick = pit_get_ticks();
    bg_loop = (loop != 0);
    bg_active = true;
    bg_note_on = false;
}

void pcspk_bg_update(void) {
    if (!bg_active || !bg_song) {
        return;
    }

    uint32_t now = pit_get_ticks();
    if ((int32_t)(now - bg_next_tick) < 0) {
        return;
    }

    if (bg_note_on) {
        stop_sound();
        bg_note_on = false;
        bg_next_tick = now + NOTE_GAP_MS;
        return;
    }

    if (bg_index >= bg_song->note_count) {
        if (bg_loop) {
            bg_index = 0;
        } else {
            pcspk_bg_stop();
            return;
        }
    }

    play_sound(bg_song->notes[bg_index].frequency_hz);
    bg_next_tick = now + bg_song->notes[bg_index].duration_ms;
    bg_index++;
    bg_note_on = true;
}

void pcspk_bg_stop(void) {
    bg_active = false;
    bg_note_on = false;
    bg_song = 0;
    bg_index = 0;
    stop_sound();
    disable_speaker();
}
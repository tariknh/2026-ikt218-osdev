#ifndef PCSPK_H
#define PCSPK_H

#include <libc/stdint.h>

struct note {
    uint32_t frequency_hz;
    uint32_t duration_ms;
};

struct song {
    const struct note *notes;
    uint32_t note_count;
    const char *name;
};

void enable_speaker(void);
void disable_speaker(void);
void play_sound(uint32_t frequency_hz);
void stop_sound(void);
void play_song(const struct song *song);
void play_demo_song(void);
void pcspk_bg_start(const struct song *song, int loop);
void pcspk_bg_update(void);
void pcspk_bg_stop(void);

#endif
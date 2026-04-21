#ifdef SOUNDPLAYER_H
#define SOUNDPLAYER_H

#include "libc/stdint.h"

static inline void outb(uint16_t port, uint8_t value);

static inline uint8_t inb(uint16_t port);

void enable_speaker();

void disable_speaker();

void play_sound(uint32_t frequency);

void stop_sound();

#endif //SOUNDPLAYER_H
#ifndef SPEAKER_H
#define SPEAKER_H

#include "stdint.h"

void enable_speaker(void);
void disable_speaker(void);
void play_sound(uint32_t frequency);
void stop_sound(void);

#endif

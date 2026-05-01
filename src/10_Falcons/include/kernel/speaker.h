#ifndef UIAOS_KERNEL_SPEAKER_H
#define UIAOS_KERNEL_SPEAKER_H

#include <stdint.h>

void enable_speaker(void);
void disable_speaker(void);
void play_sound(uint32_t frequency);
void stop_sound(void);

#endif

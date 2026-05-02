#ifndef SONG_H
#define SONG_H

#include <libc/stdint.h>
#include <libc/stddef.h>

typedef struct {
    uint32_t frequency;
    uint32_t duration;
} Note;

typedef struct {
    Note* notes;
    uint32_t length;
} Song;

#ifdef __cplusplus
extern "C" {
#endif

void enable_speaker(void);
void disable_speaker(void);
void play_sound(uint32_t frequency);
void stop_sound(void);
void play_song(Song* song);

#ifdef __cplusplus
}
#endif

#endif
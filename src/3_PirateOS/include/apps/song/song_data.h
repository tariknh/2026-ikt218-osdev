#ifndef SONG_DATA_H
#define SONG_DATA_H

#include "apps/song/song.h"

// Songs available through the music command
extern Song available_songs[];
extern const uint32_t available_song_count;

// Background music used by the raycaster
extern Note raycaster_bg_theme[];
extern const uint32_t raycaster_bg_theme_length;

#endif
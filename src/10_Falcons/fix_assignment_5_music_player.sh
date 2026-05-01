#!/usr/bin/env bash
set -euo pipefail

# Run this from: 2026-ikt218-osdev/src/10_Falcons
if [ ! -f CMakeLists.txt ] || [ ! -d src ] || [ ! -d include ]; then
  echo "ERROR: Run this script from src/10_Falcons" >&2
  exit 1
fi

mkdir -p include/song include/kernel reports

cat > include/song/frequencies.h <<'FEOF'
#ifndef UIAOS_FREQUENCIES_H
#define UIAOS_FREQUENCIES_H

#define C3 131
#define D3 147
#define E3 165
#define F3 175
#define G3 196
#define A3 220
#define B3 247

#define C4 262
#define Cs4 277
#define D4 294
#define Ds4 311
#define E4 330
#define F4 349
#define Fs4 370
#define G4 392
#define Gs4 415
#define A4 440
#define As4 466
#define B4 494

#define C5 523
#define Cs5 554
#define D5 587
#define Ds5 622
#define E5 659
#define F5 698
#define Fs5 740
#define G5 784
#define Gs5 831
#define A5 880
#define As5 932
#define B5 988

#define A_SHARP4 466
#define G_SHARP4 415
#define R 0

#endif
FEOF

cat > include/song/song.h <<'FEOF'
#ifndef UIAOS_SONG_H
#define UIAOS_SONG_H

#include <stdint.h>
#include <stddef.h>
#include <song/frequencies.h>

typedef struct Note {
    uint32_t frequency;
    uint32_t duration;
} Note;

typedef struct Song {
    Note *notes;
    uint32_t length;
} Song;

typedef struct SongPlayer {
    void (*play_song)(Song *song);
} SongPlayer;

SongPlayer *create_song_player(void);
void play_song_impl(Song *song);
void play_music(void);

static Note music_1[] = {
    {E5, 250}, {R, 125}, {E5, 125}, {R, 125}, {E5, 125}, {R, 125},
    {C5, 125}, {E5, 125}, {G5, 125}, {R, 125}, {G4, 125}, {R, 250},
    {C5, 125}, {R, 250}, {G4, 125}, {R, 125}, {E4, 125}, {R, 125},
    {A4, 125}, {B4, 125}, {R, 125}, {A_SHARP4, 125}, {A4, 125}, {R, 125},
    {G4, 125}, {E5, 125}, {G5, 125}, {A5, 125}, {F5, 125}, {G5, 125},
    {R, 125}, {E5, 125}, {C5, 125}, {D5, 125}, {B4, 125}, {R, 125},
};

static Note starwars_theme[] = {
    {A4, 500}, {A4, 500}, {A4, 500},
    {F4, 375}, {C5, 125},
    {A4, 500}, {F4, 375}, {C5, 125}, {A4, 1000},
    {E5, 500}, {E5, 500}, {E5, 500},
    {F5, 375}, {C5, 125},
    {G4, 500}, {F4, 375}, {C5, 125}, {A4, 1000},
    {R, 500}
};

#endif
FEOF

cat > include/kernel/speaker.h <<'FEOF'
#ifndef UIAOS_KERNEL_SPEAKER_H
#define UIAOS_KERNEL_SPEAKER_H

#include <stdint.h>

void enable_speaker(void);
void disable_speaker(void);
void play_sound(uint32_t frequency);
void stop_sound(void);

#endif
FEOF

cat > src/speaker.c <<'FEOF'
#include <stdint.h>
#include <kernel/io.h>
#include <kernel/speaker.h>

#define PIT_CMD_PORT        0x43
#define PIT_CHANNEL2_PORT   0x42
#define PC_SPEAKER_PORT     0x61
#define PIT_BASE_FREQUENCY  1193180

void enable_speaker(void)
{
    uint8_t value = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, value | 0x03);
}

void disable_speaker(void)
{
    uint8_t value = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, value & 0xFC);
}

void play_sound(uint32_t frequency)
{
    if (frequency == 0) {
        stop_sound();
        return;
    }

    uint32_t divisor = PIT_BASE_FREQUENCY / frequency;

    outb(PIT_CMD_PORT, 0xB6);
    outb(PIT_CHANNEL2_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL2_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    enable_speaker();
}

void stop_sound(void)
{
    uint8_t value = inb(PC_SPEAKER_PORT);
    outb(PC_SPEAKER_PORT, value & 0xFC);
}
FEOF

cat > src/song_player.c <<'FEOF'
#include <stdint.h>
#include <stddef.h>
#include <song/song.h>
#include <kernel/speaker.h>
#include <kernel/pit.h>
#include <kernel/memory.h>
#include <kernel/terminal.h>

SongPlayer *create_song_player(void)
{
    SongPlayer *player = (SongPlayer *)malloc(sizeof(SongPlayer));
    if (player == NULL) {
        return NULL;
    }

    player->play_song = play_song_impl;
    return player;
}

void play_song_impl(Song *song)
{
    if (song == NULL || song->notes == NULL) {
        return;
    }

    enable_speaker();

    for (uint32_t i = 0; i < song->length; i++) {
        Note note = song->notes[i];

        if (note.frequency == 0) {
            stop_sound();
        } else {
            play_sound(note.frequency);
        }

        sleep_interrupt(note.duration);
        stop_sound();
        sleep_busy(20);
    }

    disable_speaker();
}

void play_music(void)
{
    Song songs[] = {
        { music_1, sizeof(music_1) / sizeof(Note) },
        { starwars_theme, sizeof(starwars_theme) / sizeof(Note) }
    };

    SongPlayer *player = create_song_player();
    if (player == NULL) {
        printf("[MUSIC] Failed to create song player.\n");
        return;
    }

    uint32_t n_songs = sizeof(songs) / sizeof(Song);

    for (uint32_t i = 0; i < n_songs; i++) {
        printf("[MUSIC] Playing song %d/%d...\n", i + 1, n_songs);
        player->play_song(&songs[i]);
        printf("[MUSIC] Finished song %d.\n", i + 1);
        sleep_interrupt(500);
    }

    free(player);
    disable_speaker();
}
FEOF

# Add source files to CMakeLists.txt if they are missing.
python3 - <<'PY'
from pathlib import Path
p = Path('CMakeLists.txt')
s = p.read_text()
for src in ['src/speaker.c', 'src/song_player.c']:
    if src not in s:
        marker = 'src/pit.c'
        if marker in s:
            s = s.replace(marker, marker + '\n    ' + src, 1)
        else:
            # Fallback: insert before closing parenthesis of first add_executable block.
            idx = s.find('add_executable')
            end = s.find(')', idx)
            if idx != -1 and end != -1:
                s = s[:end] + f'\n    {src}\n' + s[end:]
p.write_text(s)
PY

# Add song header and call play_music() once after PIT initialization/test area.
python3 - <<'PY'
from pathlib import Path
p = Path('src/kernel.c')
s = p.read_text()
if '#include <song/song.h>' not in s and '#include "song/song.h"' not in s:
    lines = s.splitlines()
    insert_at = 0
    for i,l in enumerate(lines):
        if l.startswith('#include'):
            insert_at = i + 1
    lines.insert(insert_at, '#include <song/song.h>')
    s = '\n'.join(lines) + '\n'
if 'play_music();' not in s:
    target = 'printf("[PIT] Interrupt sleep OK.\\n");'
    if target in s:
        s = s.replace(target, target + '\n\n    printf("[MUSIC] Starting PC speaker music test...\\n");\n    play_music();\n    printf("[MUSIC] Music test completed.\\n");', 1)
    else:
        # Fallback: insert before the first final infinite loop/return if possible.
        marker = 'while (1)'
        if marker in s:
            s = s.replace(marker, 'printf("[MUSIC] Starting PC speaker music test...\\n");\n    play_music();\n    printf("[MUSIC] Music test completed.\\n");\n\n    ' + marker, 1)
        else:
            s += '\n/* Assignment 5 music test */\nvoid assignment5_note(void) { play_music(); }\n'
p.write_text(s)
PY

cat > reports/assignment_5_music_player.md <<'FEOF'
# Assignment 5 - Music Player

Implemented PC speaker output using PIT channel 2 and port 0x61. The code adds a speaker driver, song data, a SongPlayer structure, and a play_music test function. The PIT from Assignment 4 is reused for note timing.
FEOF

echo "Done: Assignment 5 music player files were added."
echo
printf '%s\n' "Now test:" \
"  rm -rf build" \
"  cmake -S . -B build" \
"  cmake --build build" \
"  cmake --build build --target uiaos-create-image" \
"  qemu-system-i386 -cdrom build/kernel.iso -audiodev pa,id=snd0 -machine pcspk-audiodev=snd0" \
"" \
"If the audio backend fails in WSL, first try the normal command:" \
"  qemu-system-i386 -cdrom build/kernel.iso" \
"" \
"Expected screen output includes:" \
"  [MUSIC] Starting PC speaker music test..." \
"  [MUSIC] Playing song 1/2..." \
"  [MUSIC] Finished song 1."

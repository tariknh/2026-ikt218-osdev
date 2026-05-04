#ifndef PIANO_H
#define PIANO_H

#include "libc/stdint.h"
#include "pit.h"

// Note frequencies (Hz) — one octave
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523

void piano_init(void);
void piano_play_tone(uint32_t frequency, uint32_t duration_ms);
void piano_stop_tone(void);

void piano_run(void);

// Funksjonsdeklarasjoner for piano og port I/O
void play_sound(uint32_t nFrequence);
void nosound(void);
void beep(void);
void timer_wait(uint32_t ms);
uint8_t inb(uint16_t port);
void outb(uint16_t port, uint8_t value);

#endif

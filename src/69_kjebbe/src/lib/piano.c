#include "../../include/piano.h"
#include "../../include/kernel/pit.h"
#include "../../include/keyboard.h"
#include "../../include/libc/stdio.h"
#include "../../include/matrix.h"
#include "song/frequencies.h"
#include "song/song.h"

// Defines piano keyboard layout and their offsets from C.
static const uint32_t scancode_note_offsets[128] = {
    [0x1E] = 1,  // a (c)
    [0x11] = 2,  // w (Db)
    [0x1F] = 3,  // s (d)
    [0x12] = 4,  // e (Eb)
    [0x20] = 5,  // d (e)
    [0x21] = 6,  // f (f)
    [0x14] = 7,  // t (f#)
    [0x22] = 8,  // g (g)
    [0x15] = 9,  // y (Ab)
    [0x23] = 10, // h (A)
    [0x16] = 11, // u (Bb)
    [0x24] = 12, // j (H)
    [0x25] = 13, // k (C)
    [0x18] = 14, // o (Db)
    [0x26] = 15, // l (d)
};

void piano_keyboard_handler(uint8_t scancode) {
  // if key release bit is set we disable speaker and return
  if (scancode & 0x80) {
    disable_speaker();
    return;
  }
  int init_offset = scancode_note_offsets[scancode];
  if (init_offset) {
    int note_offset = init_offset - 1;
    int octave = 5;
    int freq_to_play = (octave * 12) + note_offset;
    int freq = freqs[freq_to_play];
    // Changes the letter and number that is drawn on the matrix.
    update_matrix_current(note_offset);
    piano_play_sound(freq);
  }
}

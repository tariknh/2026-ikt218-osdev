#include "piano.h"
#include "irq.h"
#include "pcspk.h"
#include "pit.h"
#include "terminal.h"
#include <libc/stdint.h>

// Run the small keyboard piano
void piano_run(void) {
    // Notes for one octave on the PC speaker
    static const uint32_t piano_freq[8] = {
        262, 294, 330, 349, 392, 440, 494, 523
    };

    // Show a short help line on screen
    terminal_printf("Piano mode: keys 1..8 (C D E F G A B C), ESC/q exits\n");
    for (;;) {
        // Wait for one key from the keyboard buffer
        int c = kbd_getchar();
        if (c < 0) continue;

        // ESC or q stops the piano
        if (c == 27 || c == 'q' || c == 'Q') {
            stop_sound();
            disable_speaker();
            terminal_printf("Exit piano mode\n");
            return;
        }

        // Keys 1 to 8 play one note each
        if (c >= '1' && c <= '8') {
            uint32_t f = piano_freq[(uint32_t)(c - '1')];
            play_sound(f);
            sleep_interrupt(140);
            stop_sound();
            sleep_interrupt(20);
        }
    }
}
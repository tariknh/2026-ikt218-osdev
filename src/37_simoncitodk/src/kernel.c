#include "gdt.h"
#include "idt.h"
#include "memory.h"
#include "pic.h"
#include "pit.h"
#include "song.h"
#include "terminal.h"
#include <libc/stdint.h>

extern uint32_t end;
extern uint32_t *test_new_operator(void);

void kernel_main(void)
{
    gdt_init();
    idt_init();
    pic_remap();
    pic_mask_all_except_timer_and_keyboard();

    init_kernel_memory(&end);
    init_paging();

    void *some_memory = malloc(12345);
    void *memory2 = malloc(54321);
    void *memory3 = malloc(13331);

    free(memory2);
    void *memory4 = malloc(1000);

    uint32_t *new_value = test_new_operator();

    init_pit();

    terminal_write("Memory manager test\n");

    terminal_write("malloc 1: ");
    terminal_write_hex((uint32_t)some_memory);
    terminal_write("\n");

    terminal_write("malloc 2: ");
    terminal_write_hex((uint32_t)memory2);
    terminal_write("\n");

    terminal_write("malloc 3: ");
    terminal_write_hex((uint32_t)memory3);
    terminal_write("\n");

    terminal_write("reuse after free: ");
    terminal_write_hex((uint32_t)memory4);
    terminal_write("\n");

    terminal_write("new uint32_t: ");
    terminal_write_hex((uint32_t)new_value);
    terminal_write(" = ");
    terminal_write_dec(*new_value);
    terminal_write("\n");

    print_memory_layout();

    terminal_write("Busy sleep start\n");
    asm volatile("sti");
    sleep_busy(2000);
    terminal_write("Busy sleep done\n");

    terminal_write("Interrupt sleep start\n");
    sleep_interrupt(2000);
    terminal_write("Interrupt sleep done\n");

    const uint32_t tempo = 114;
    const uint32_t whole_note_ms = (60000 * 4) / tempo;
    const uint32_t half_note_ms = whole_note_ms / 2;
    const uint32_t quarter_note_ms = whole_note_ms / 4;
    const uint32_t eighth_note_ms = whole_note_ms / 8;
    const uint32_t sixteenth_note_ms = whole_note_ms / 16;
    const uint32_t dotted_quarter_note_ms = quarter_note_ms + eighth_note_ms;
    const uint32_t dotted_eighth_note_ms = eighth_note_ms + sixteenth_note_ms;

    const uint32_t half_sound_ms = (half_note_ms * 6) / 10;
    const uint32_t quarter_sound_ms = (quarter_note_ms * 6) / 10;
    const uint32_t eighth_sound_ms = (eighth_note_ms * 6) / 10;
    const uint32_t sixteenth_sound_ms = (sixteenth_note_ms * 6) / 10;
    const uint32_t dotted_quarter_sound_ms = (dotted_quarter_note_ms * 6) / 10;
    const uint32_t dotted_eighth_sound_ms = (dotted_eighth_note_ms * 6) / 10;

    const uint32_t half_pause_ms = half_note_ms - half_sound_ms;
    const uint32_t quarter_pause_ms = quarter_note_ms - quarter_sound_ms;
    const uint32_t eighth_pause_ms = eighth_note_ms - eighth_sound_ms;
    const uint32_t sixteenth_pause_ms = sixteenth_note_ms - sixteenth_sound_ms;
    const uint32_t dotted_quarter_pause_ms = dotted_quarter_note_ms - dotted_quarter_sound_ms;
    const uint32_t dotted_eighth_pause_ms = dotted_eighth_note_ms - dotted_eighth_sound_ms;

    Note never_gonna_chorus[] = {
        {A4, sixteenth_sound_ms}, {R, sixteenth_pause_ms},
        {B4, sixteenth_sound_ms}, {R, sixteenth_pause_ms},
        {D5, sixteenth_sound_ms}, {R, sixteenth_pause_ms},
        {B4, sixteenth_sound_ms}, {R, sixteenth_pause_ms},
        {Fs5, dotted_eighth_sound_ms}, {R, dotted_eighth_pause_ms},
        {Fs5, dotted_eighth_sound_ms}, {R, dotted_eighth_pause_ms},
        {E5, dotted_quarter_sound_ms}, {R, dotted_quarter_pause_ms},

        {A4, sixteenth_sound_ms}, {R, sixteenth_pause_ms},
        {B4, sixteenth_sound_ms}, {R, sixteenth_pause_ms},
        {D5, sixteenth_sound_ms}, {R, sixteenth_pause_ms},
        {B4, sixteenth_sound_ms}, {R, sixteenth_pause_ms},
        {E5, dotted_eighth_sound_ms}, {R, dotted_eighth_pause_ms},
        {E5, dotted_eighth_sound_ms}, {R, dotted_eighth_pause_ms},
        {D5, dotted_eighth_sound_ms}, {R, dotted_eighth_pause_ms},
        {Cs5, sixteenth_sound_ms}, {R, sixteenth_pause_ms},
        {B4, dotted_eighth_sound_ms}, {R, dotted_eighth_pause_ms},

        {A4, sixteenth_sound_ms}, {R, sixteenth_pause_ms},
        {B4, sixteenth_sound_ms}, {R, sixteenth_pause_ms},
        {D5, sixteenth_sound_ms}, {R, sixteenth_pause_ms},
        {B4, sixteenth_sound_ms}, {R, sixteenth_pause_ms},
        {D5, quarter_sound_ms}, {R, quarter_pause_ms},
        {E5, eighth_sound_ms}, {R, eighth_pause_ms},
        {Cs5, dotted_eighth_sound_ms}, {R, dotted_eighth_pause_ms},
        {B4, sixteenth_sound_ms}, {R, sixteenth_pause_ms},
        {A4, eighth_sound_ms}, {R, eighth_pause_ms},
        {A4, eighth_sound_ms}, {R, eighth_pause_ms},
        {A4, eighth_sound_ms}, {R, eighth_pause_ms},
        {E5, quarter_sound_ms}, {R, quarter_pause_ms},
        {D5, half_sound_ms}, {R, half_pause_ms},
        {R, 1000}
    };

    Song test_song = {
        never_gonna_chorus,
        sizeof(never_gonna_chorus) / sizeof(Note)
    };

    SongPlayer *player = create_song_player();

    if (player != 0) {
        player->play_song(&test_song);
    } else {
        terminal_write("Could not create song player\n");
    }

    terminal_write("Press keys to trigger IRQ1\n");

    for (;;) {
        asm volatile("hlt");
    }
}

#include "final_demo.h"
#include "idt.h"
#include "memory.h"
#include "pit.h"
#include "song.h"
#include "terminal.h"
#include <libc/stdint.h>

extern uint32_t *test_new_operator(void);

static void final_demo_pause(uint32_t milliseconds)
{
    sleep_interrupt(milliseconds);
}

static void final_demo_type_text(const char *text)
{
    int index = 0;

    while (text[index] != '\0') {
        char character[2] = {text[index], '\0'};
        terminal_write(character);

        if (text[index] == '\n') {
            sleep_interrupt(350);
        } else {
            sleep_interrupt(30);
        }

        index++;
    }
}

static int final_demo_string_equals(const char *left, const char *right)
{
    int index = 0;

    while (left[index] != '\0' && right[index] != '\0') {
        if (left[index] != right[index]) {
            return 0;
        }

        index++;
    }

    return left[index] == '\0' && right[index] == '\0';
}

static char final_demo_wait_for_key_change(char previous_key)
{
    char current_key = keyboard_get_last_key();

    while (current_key == previous_key) {
        sleep_interrupt(30);
        current_key = keyboard_get_last_key();
    }

    return current_key;
}

static void final_demo_show_boot(void)
{
    terminal_clear();
    final_demo_type_text("simoncitodk-os\n\n");
    final_demo_type_text("Hello World\n\n");
    final_demo_type_text("Multiboot2 kernel loaded\n");
    final_demo_type_text("GDT loaded\n");
    final_demo_type_text("VGA text terminal ready\n");

    final_demo_pause(1500);
}

static void final_demo_show_keyboard_input(void)
{
    char input[8];
    char previous_key = keyboard_get_last_key();

    terminal_clear();
    final_demo_type_text("Interrupt and keyboard input demo\n\n");
    final_demo_type_text("IDT installed\n");
    final_demo_type_text("PIC remapped\n");
    final_demo_type_text("Keyboard IRQ1 enabled\n\n");
    final_demo_type_text("Type start and press ENTER to continue:\n\n> ");

    for (;;) {
        int input_length = 0;

        while (1) {
            char key = final_demo_wait_for_key_change(previous_key);
            previous_key = key;

            if (key == '\n') {
                input[input_length] = '\0';
                terminal_write("\n");
                break;
            }

            if (key == '\b') {
                if (input_length > 0) {
                    input_length--;
                    terminal_write("\n> ");
                    for (int i = 0; i < input_length; i++) {
                        char text[2] = {input[i], '\0'};
                        terminal_write(text);
                    }
                }
                continue;
            }

            if (input_length < 7) {
                input[input_length] = key;
                input_length++;

                char text[2] = {key, '\0'};
                terminal_write(text);
            }
        }

        if (final_demo_string_equals(input, "start")) {
            final_demo_type_text("Keyboard input accepted\n");
            final_demo_pause(1800);
            return;
        }

        final_demo_type_text("Not accepted. Try again:\n> ");
    }
}
static void final_demo_show_memory(void)
{
    terminal_clear();
    final_demo_type_text("Memory manager and paging\n\n");

    void *some_memory = malloc(12345);
    void *memory2 = malloc(54321);
    void *memory3 = malloc(13331);

    free(memory2);
    void *memory4 = malloc(1000);

    uint32_t *new_value = test_new_operator();

    final_demo_type_text("Paging enabled\n");

    final_demo_type_text("malloc 1: ");
    terminal_write_hex((uint32_t)some_memory);
    final_demo_type_text("\n");

    final_demo_type_text("malloc 2: ");
    terminal_write_hex((uint32_t)memory2);
    final_demo_type_text("\n");

    final_demo_type_text("malloc 3: ");
    terminal_write_hex((uint32_t)memory3);
    final_demo_type_text("\n");

    final_demo_type_text("free malloc 2\n");

    final_demo_type_text("reuse after free: ");
    terminal_write_hex((uint32_t)memory4);
    final_demo_type_text("\n");

    final_demo_type_text("C++ new uint32_t: ");
    terminal_write_hex((uint32_t)new_value);
    final_demo_type_text(" = ");
    terminal_write_dec(*new_value);
    final_demo_type_text("\n");

    final_demo_pause(4000);
}
static void final_demo_show_timer(void)
{
    terminal_clear();
    final_demo_type_text("PIT timer and sleep functions\n\n");
    final_demo_type_text("PIT frequency: 1000 Hz\n\n");

    final_demo_type_text("Busy sleep start\n");
    sleep_busy(2000);
    final_demo_type_text("Busy sleep done\n\n");

    final_demo_type_text("Interrupt sleep start\n");
    sleep_interrupt(2000);
    final_demo_type_text("Interrupt sleep done\n");

    final_demo_pause(1500);
}

static void final_demo_play_pc_speaker_song(void)
{
    terminal_clear();
    final_demo_type_text("PC speaker music\n\n");
    final_demo_type_text("PIT channel 2 drives the speaker frequency.\n");
    final_demo_type_text("Interrupt sleep controls note timing.\n\n");

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

    Song song = {
        never_gonna_chorus,
        sizeof(never_gonna_chorus) / sizeof(Note)
    };

    SongPlayer *player = create_song_player();

    if (player != 0) {
        player->play_song(&song);
    } else {
        terminal_write("Could not create song player\n");
    }

    final_demo_pause(1500);
}

static void final_demo_start_matrix_rain(void)
{
    terminal_clear();
    final_demo_type_text("Matrix rain\n\n");
    final_demo_type_text("VGA text mode animation\n");
    final_demo_type_text("PIT frame timing\n");
    final_demo_type_text("Keyboard speed control\n\n");
    final_demo_type_text("Controls: 1 = slow, 2 = normal, 3 = fast\n");
    final_demo_type_text("Starting Matrix rain...\n");

    final_demo_pause(2000);
}

void final_demo_run(void)
{
    final_demo_show_boot();
    final_demo_play_pc_speaker_song();
    final_demo_show_keyboard_input();
    final_demo_show_memory();
    final_demo_show_timer();
    final_demo_start_matrix_rain();
}
void final_demo_show_complete(void)
{
    terminal_clear();
    final_demo_type_text("simoncitodk-os demo complete\n\n");
    final_demo_type_text("The kernel is now halted.\n");

    for (;;) {
        asm volatile("hlt");
    }
}

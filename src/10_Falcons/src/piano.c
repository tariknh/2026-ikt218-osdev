#include <kernel/piano.h>
#include <kernel/speaker.h>
#include <kernel/terminal.h>
#include <stdint.h>

typedef struct {
    uint8_t scancode;
    uint32_t frequency;
    const char *name;
} piano_key_t;

static piano_key_t piano_keys[] = {
    {0x02, 262, "C4"},
    {0x03, 294, "D4"},
    {0x04, 330, "E4"},
    {0x05, 349, "F4"},
    {0x06, 392, "G4"},
    {0x07, 440, "A4"},
    {0x08, 494, "B4"},
    {0x09, 523, "C5"}
};

void piano_init(void)
{
    printf("[IMPROV] Piano mode enabled.\n");
    printf("[IMPROV] Press keys 1-8 to play notes.\n");
    printf("[IMPROV] 1=C 2=D 3=E 4=F 5=G 6=A 7=B 8=C5\n");
}

void piano_handle_scancode(uint8_t scancode)
{
    if (scancode & 0x80) {
        stop_sound();
        return;
    }

    for (uint32_t i = 0; i < sizeof(piano_keys) / sizeof(piano_keys[0]); i++) {
        if (piano_keys[i].scancode == scancode) {
            printf("[IMPROV] Playing note ");
            printf(piano_keys[i].name);
            printf(" at %d Hz\n", piano_keys[i].frequency);

            play_sound(piano_keys[i].frequency);
            return;
        }
    }
}

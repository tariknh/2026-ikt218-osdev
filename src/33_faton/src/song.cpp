extern "C" {
    #include <song.h>
    #include <pit.h>
    #include <monitor.h>
    #include <libc/stdint.h>
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void enable_speaker(void) {
    uint8_t tmp = inb(0x61);
    if ((tmp & 3) != 3) {
        outb(0x61, tmp | 3);
    }
}

void disable_speaker(void) {
    uint8_t tmp = inb(0x61);
    outb(0x61, tmp & 0xFC);
}

void play_sound(uint32_t frequency) {
    if (frequency == 0) return;

    uint32_t div = 1193180 / frequency;
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)(div & 0xFF));
    outb(0x42, (uint8_t)((div >> 8) & 0xFF));

    enable_speaker();
}

void stop_sound(void) {
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

void play_song(Song* song) {
    enable_speaker();

    for (uint32_t i = 0; i < song->length; i++) {
        Note* note = &song->notes[i];
        printf("Playing note: %d Hz for %d ms\n",
               note->frequency, note->duration);
        play_sound(note->frequency);
        sleep_interrupt(note->duration);
        stop_sound();
    }

    disable_speaker();
}
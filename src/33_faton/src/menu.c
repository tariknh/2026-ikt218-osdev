#include <menu.h>
#include <monitor.h>
#include <pit.h>
#include <memory.h>
#include <song.h>
#include <frequencies.h>
#include <keyboard.h>
#include <libc/stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}

static Note music_1[] = {
    {E5, 125}, {R, 125}, {E5, 125}, {R, 125},
    {E5, 125}, {R, 125}, {C5, 125}, {E5, 125},
    {G5, 250}, {R, 250}, {G4, 250}, {R, 250},
    {C5, 250}, {R, 125}, {G4, 125}, {R, 250},
    {E4, 250}, {R, 125}, {A4, 125}, {B4, 125},
    {R, 125},  {A4, 125},{G4, 125}, {E5, 125},
    {G5, 125}, {A5, 125},{F5, 125}, {G5, 125},
    {R, 125},  {E5, 125},{C5, 125}, {D5, 125},
    {B4, 250}, {R, 250},
};

static void beep(uint32_t freq, uint32_t duration) {
    play_sound(freq);
    sleep_interrupt(duration);
    stop_sound();
}

void shutdown(void) {
    monitor_clear();
    monitor_set_color(COLOR_RED, COLOR_BLACK);
    printf("Shutting down FatonOS...\n");
    monitor_set_color(COLOR_WHITE, COLOR_BLACK);
    sleep_interrupt(1000);
    outb(0x64, 0xFE);
    __asm__ __volatile__("cli");
    for (;;) __asm__ __volatile__("hlt");
}

void run_piano(void) {
    monitor_clear();
    monitor_set_color(COLOR_CYAN, COLOR_BLACK);
    printf("=== PIANO MODE ===\n\n");
    monitor_set_color(COLOR_GREEN, COLOR_BLACK);
    printf("A=C4  S=D4  D=E4  F=F4\n");
    printf("G=G4  H=A4  J=B4  K=C5\n\n");
    monitor_set_color(COLOR_WHITE, COLOR_BLACK);
    printf("Press ESC to go back\n\n");

    while (1) {
        char k = last_key;
        last_key = 0;

        if (k == 27) break;

        uint32_t freq = 0;
        const char* note_name = 0;

        if      (k == 'a') { freq = C4; note_name = "C4"; }
        else if (k == 's') { freq = D4; note_name = "D4"; }
        else if (k == 'd') { freq = E4; note_name = "E4"; }
        else if (k == 'f') { freq = F4; note_name = "F4"; }
        else if (k == 'g') { freq = G4; note_name = "G4"; }
        else if (k == 'h') { freq = A4; note_name = "A4"; }
        else if (k == 'j') { freq = B4; note_name = "B4"; }
        else if (k == 'k') { freq = C5; note_name = "C5"; }

        if (freq != 0) {
            monitor_set_color(COLOR_YELLOW, COLOR_BLACK);
            printf("Playing: %s (%d Hz)\n", note_name, freq);
            monitor_set_color(COLOR_WHITE, COLOR_BLACK);
            beep(freq, 200);
        }

        __asm__ __volatile__("hlt");
    }

    monitor_clear();
}

void run_system_info(void) {
    monitor_clear();
    monitor_set_color(COLOR_CYAN, COLOR_BLACK);
    printf("=== SYSTEM INFO ===\n\n");
    monitor_set_color(COLOR_WHITE, COLOR_BLACK);
    print_memory_layout();
    printf("\nPIT frequency: 1000 Hz\n");
    printf("PIT ticks: %d\n", get_current_tick());
    printf("\nPress any key to go back...\n");
    last_key = 0;
    while (last_key == 0) {
        __asm__ __volatile__("hlt");
    }
    last_key = 0;
    monitor_clear();
}

void run_menu(void) {
    while (1) {
        monitor_clear();

        monitor_set_color(COLOR_CYAN, COLOR_BLACK);
        printf("================================\n");
        printf("      Welcome to FatonOS!       \n");
        printf("================================\n\n");

        monitor_set_color(COLOR_GREEN, COLOR_BLACK);
        printf("1. Play music\n");
        printf("2. Piano\n");
        printf("3. System info\n");
        printf("4. Shutdown\n\n");

        monitor_set_color(COLOR_WHITE, COLOR_BLACK);
        printf("Enter choice: ");

        char choice = 0;
        while (choice == 0) {
            choice = last_key;
            last_key = 0;
            __asm__ __volatile__("hlt");
        }

        monitor_set_color(COLOR_WHITE, COLOR_BLACK);

        if (choice == '1') {
            monitor_clear();
            printf("Playing song...\n");
            Song song1 = {music_1, sizeof(music_1) / sizeof(Note)};
            play_song(&song1);
            printf("Song finished!\n");
            sleep_interrupt(1000);
        } else if (choice == '2') {
            run_piano();
        } else if (choice == '3') {
            run_system_info();
        } else if (choice == '4') {
            shutdown();
        }
    }
}
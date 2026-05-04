#include "kernel/apps.h"

#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"

void run_shutdown(void) {
    terminal_clear();
    printf("Shutting down...\n");

    // Writes QEMU shutdown command to the power-off I/O ports
    __asm__ volatile ("outw %0, %1" : : "a"((uint16_t) 0x2000), "Nd"((uint16_t) 0x604));
    __asm__ volatile ("outw %0, %1" : : "a"((uint16_t) 0x2000), "Nd"((uint16_t) 0xB004));

    // If the emulator ignores the shutdown, then just halt the CPU
    while (true) {
        __asm__ volatile ("cli; hlt");
    }
}

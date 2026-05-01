#include "libc/stdint.h"
#include "terminal.h"
#include "gdt.h"
#include "idt.h"
#include "io.h"
#include "irq.h"
#include "isr.h"
#include "keyboard.h"

void kmain(uint32_t magic, void* mb_info_addr) {
    (void)magic;
    (void)mb_info_addr;

    init_gdt();
    terminal_initialize();
    printf("Hello World\n");

    printf("Installing IDT...\n");
    printf("Address of irq1: ");
    print_int((uint32_t)irq1);
    printf("\n");

    idt_install();
    idt_print_entry(33);

    uint16_t idt_limit;
    __asm__ __volatile__("sidt %0" : "=m"(idt_limit));
    printf("IDT limit: ");
    print_int(idt_limit);
    printf("\n");



    printf("Installing IRQ...\n");
    irq_install();
    print("Master mask: "); print_int(inb(0x21)); print("\n");
    print("Slave mask: "); print_int(inb(0xA1)); print("\n");

    printf("Registering keyboard handler...\n");
    irq_install_handler(1, keyboard_handler);

    outb(0x64, 0x20);                // Read command byte
    uint8_t cmd = inb(0x60);
    cmd |= 0x01;                     // Set bit 0: enable interrupt
    outb(0x64, 0x60);                // Write command byte
    outb(0x60, cmd);
    print("Keyboard interrupt enabled.\n");

    // Also ensure keyboard itself is enabled
    outb(0x64, 0xAE);
    print("Keyboard controller enabled.\n");
    printf("Enabling interrupts...\n");
    __asm__ __volatile__("sti");

    printf("\n=== OS Ready ===\n");
    printf("Type on your keyboard. Characters will appear below:\n");
    printf("----------------------------------------\n");

    print("Polling keyboard for 10 seconds. Press keys...\n");
    for (int i = 0; i < 10000000; i++) {
        if (inb(0x64) & 0x01) {            // Data available?
            uint8_t scancode = inb(0x60);
            print("Scancode: ");
            print_int(scancode);
            print("\n");
            // If you see scancodes, the keyboard hardware works!
        }
        // Small delay to avoid burning CPU
        for (int d = 0; d < 10; d++) __asm__("nop");
    }
    print("Polling finished.\n");
    while (1) {
        __asm__ __volatile__("hlt");
    }
}
#include <libc/stddef.h>
#include <libc/stdint.h>
#include <gdt.h>
#include <terminal.h>
#include <idt.h>
#include <irq.h>
#include <memory.h>
#include <paging.h>
#include <pit.h>

/*
 * The end of kernel image in memory.
 */
extern uint32_t end;

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY 0xB8000

static uint16_t* const VGA_BUFFER = (uint16_t*) VGA_MEMORY;

static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = 0x0F;

static uint16_t vga_entry(unsigned char character, uint8_t color)
{
    return (uint16_t) character | (uint16_t) color << 8;
}

void terminal_initialize(void)
{
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = 0x0F;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_BUFFER[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_putchar(char character)
{
    if (character == '\n') {
        terminal_column = 0;
        terminal_row++;

        if (terminal_row == VGA_HEIGHT) {
            terminal_row = 0;
        }

        return;
    }

    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    VGA_BUFFER[index] = vga_entry(character, terminal_color);

    terminal_column++;

    if (terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        terminal_row++;

        if (terminal_row == VGA_HEIGHT) {
            terminal_row = 0;
        }
    }
}

void terminal_write(const char* data)
{
    for (size_t i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
}

void terminal_write_dec(uint32_t value)
{
    char buffer[11];
    int index = 0;

    if (value == 0) {
        terminal_putchar('0');
        return;
    }

    while (value > 0 && index < 10) {
        buffer[index] = '0' + (value % 10);
        value /= 10;
        index++;
    }

    while (index > 0) {
        index--;
        terminal_putchar(buffer[index]);
    }
}

void terminal_write_hex(uint32_t value)
{
    const char* hex_digits = "0123456789ABCDEF";

    terminal_write("0x");

    for (int shift = 28; shift >= 0; shift -= 4) {
        uint8_t digit = (uint8_t)((value >> shift) & 0xF);
        terminal_putchar(hex_digits[digit]);
    }
}

void main(void)
{
    gdt_initialize();
    terminal_initialize();

    terminal_write("UiA OS\n");
    terminal_write("Memory and PIT test\n\n");

    idt_initialize();
    irq_initialize();

    terminal_write("Initializing kernel memory...\n");
    init_kernel_memory(&end);

    terminal_write("Initializing paging...\n");
    init_paging();

    terminal_write("\nMemory layout before malloc:\n");
    print_memory_layout();

    terminal_write("\nTesting malloc:\n");

    void* memory1 = malloc(12345);
    void* memory2 = malloc(54321);
    void* memory3 = malloc(13331);

    terminal_write("  malloc(12345): ");
    terminal_write_hex((uint32_t)memory1);
    terminal_putchar('\n');

    terminal_write("  malloc(54321): ");
    terminal_write_hex((uint32_t)memory2);
    terminal_putchar('\n');

    terminal_write("  malloc(13331): ");
    terminal_write_hex((uint32_t)memory3);
    terminal_putchar('\n');

    terminal_write("\nMemory layout after malloc:\n");
    print_memory_layout();

    terminal_write("\nInitializing PIT...\n");
    init_pit();

    /*
     * Enables hardware interrupts so IRQ0 from PIT can update ticks.
     */
    __asm__ volatile ("sti");

    terminal_write("\nTesting PIT sleep functions:\n");

    uint32_t counter = 0;

    while (1) {
        terminal_write("[");
        terminal_write_dec(counter);
        terminal_write("] Sleeping with busy-waiting...\n");

        sleep_busy(1000);

        terminal_write("[");
        terminal_write_dec(counter);
        terminal_write("] Slept using busy-waiting.\n");
        counter++;

        terminal_write("[");
        terminal_write_dec(counter);
        terminal_write("] Sleeping with interrupts...\n");

        sleep_interrupt(1000);

        terminal_write("[");
        terminal_write_dec(counter);
        terminal_write("] Slept using interrupts.\n");
        counter++;
    }
}

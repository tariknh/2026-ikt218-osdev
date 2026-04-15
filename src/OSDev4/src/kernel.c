#include <stdint.h>
#include <stddef.h>
#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "serial.h"
#include "memory.h"
#include "pit.h"
#include "printf.h"

extern uint32_t end;

#define VGA_BUFFER  ((volatile uint16_t*)0xB8000)
#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_COLOR   0x0F

static size_t terminal_col = 0;
static size_t terminal_row = 0;

static void terminal_clear(void)
{
    for (size_t y = 0; y < VGA_HEIGHT; y++)
        for (size_t x = 0; x < VGA_WIDTH; x++)
            VGA_BUFFER[y * VGA_WIDTH + x] = ' ' | ((uint16_t)VGA_COLOR << 8);
    terminal_col = 0;
    terminal_row = 0;
}

void terminal_putchar(char c)
{
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
        return;
    }

    if (c == '\b') {
        if (terminal_col > 0) {
            terminal_col--;
        }
        VGA_BUFFER[terminal_row * VGA_WIDTH + terminal_col] =
            ' ' | ((uint16_t)VGA_COLOR << 8);
        return;
    }

    VGA_BUFFER[terminal_row * VGA_WIDTH + terminal_col] =
        (uint16_t)(unsigned char)c | ((uint16_t)VGA_COLOR << 8);

    if (++terminal_col >= VGA_WIDTH) {
        terminal_col = 0;
        terminal_row++;
    }

    if (terminal_row >= VGA_HEIGHT) {
        terminal_row = 0;
        terminal_col = 0;
    }
}

void terminal_write(const char* str)
{
    for (size_t i = 0; str[i] != '\0'; i++) {
        terminal_putchar(str[i]);
        serial_putchar(str[i]);
    }
}

int main(uint32_t magic, void* mboot_info)
{
    (void)magic;
    (void)mboot_info;

    serial_init();
    terminal_clear();

    gdt_init();

    idt_init();

    __asm__ volatile ("sti");

    init_kernel_memory(&end);
    init_paging();
    print_memory_layout();
    init_pit();

    terminal_write("Hello World\n");
    terminal_write("GDT initialized: NULL / Code / Data descriptors loaded.\n");
    terminal_write("IDT initialized. Interrupts enabled.\n");

    void* some_memory = malloc(12345);
    void* memory2 = malloc(54321);
    void* memory3 = malloc(13331);
    (void)some_memory;
    (void)memory2;
    (void)memory3;
    terminal_write("Memory allocated successfully.\n");

    int counter = 0;
    while(1) {
        printf("[%d]: Sleeping with busy-waiting (HIGH CPU).\n", counter);
        sleep_busy(1000);
        printf("[%d]: Slept using busy-waiting.\n", counter++);

        printf("[%d]: Sleeping with interrupts (LOW CPU).\n", counter);
        sleep_interrupt(1000);
        printf("[%d]: Slept using interrupts.\n", counter++);
    }

    return 0;
}

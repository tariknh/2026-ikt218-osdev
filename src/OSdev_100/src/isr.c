#include "../include/isr.h"
#include "../include/pit.h"
#include "../include/pic.h"      // For pic_send_eoi
#include "../include/io.h"       // For inb (if still needed here)
#include "../include/keyboard.h" // For keyboard_handler()

static int interrupt_demo_enabled = 0;
static int interrupt_demo_pending = 0;
static registers_t interrupt_demo_regs;

static void exception_write(const char* text, int row, int column, char color) {
    volatile char* video_memory = (volatile char*)0xb8000;
    int offset = (row * 80 + column) * 2;

    while (*text) {
        video_memory[offset] = *text++;
        video_memory[offset + 1] = color;
        offset += 2;
    }
}

static void exception_clear_row(int row, char color) {
    volatile char* video_memory = (volatile char*)0xb8000;
    int offset = row * 80 * 2;

    for (int column = 0; column < 80; column++) {
        video_memory[offset] = ' ';
        video_memory[offset + 1] = color;
        offset += 2;
    }
}

static void exception_clear_screen(char color) {
    for (int row = 0; row < 25; row++) {
        exception_clear_row(row, color);
    }
}

static void exception_write_dec(uint32_t value, int row, int column, char color) {
    char buffer[11];
    int index = 0;
    volatile char* video_memory = (volatile char*)0xb8000;
    int offset = (row * 80 + column) * 2;

    if (value == 0) {
        video_memory[offset] = '0';
        video_memory[offset + 1] = color;
        return;
    }

    while (value > 0) {
        buffer[index++] = (char)('0' + (value % 10));
        value /= 10;
    }

    while (index > 0) {
        video_memory[offset] = buffer[--index];
        video_memory[offset + 1] = color;
        offset += 2;
    }
}

void interrupt_demo_enable(void) {
    interrupt_demo_enabled = 1;
    interrupt_demo_pending = 0;
}

void interrupt_demo_disable(void) {
    interrupt_demo_enabled = 0;
    interrupt_demo_pending = 0;
}

int interrupt_demo_take_snapshot(registers_t* out_regs) {
    if (!interrupt_demo_pending || !out_regs) {
        return 0;
    }

    *out_regs = interrupt_demo_regs;
    interrupt_demo_pending = 0;
    return 1;
}

void exception_handler(registers_t *regs) {
    // --- 1. Handle Hardware Interrupts (IRQs 32-47) ---
    if (regs->int_no >= 32 && regs->int_no <= 47) {
        
        // --- FIXED: Handle the System Timer (Interrupt 32) ---
        if (regs->int_no == 32) {
            timer_handler(); // 1. Increment the clock (tick++)
            pic_send_eoi(0); // 2. Tell PIC we are done
            return;          // 3. Go back to work
        }

        // Check specifically for the Keyboard (Interrupt 33)
        if (regs->int_no == 33) {
            
            keyboard_handler(); // This will read the scancode and print the character

            pic_send_eoi(1); // Keyboard is IRQ 1
            return;
        }

        // Tell the PIC we are finished for any other hardware IRQs
        pic_send_eoi(regs->int_no - 32);
        return; 

        
    }

    // --- 2. Handle CPU Exceptions (0-31) ---
    // This part only runs for real CPU exceptions.
    if (interrupt_demo_enabled) {
        interrupt_demo_regs = *regs;
        interrupt_demo_pending = 1;
        return;
    }

    exception_clear_screen(0x07);
    exception_write("EXCEPTION CAUGHT", 1, 0, 0x04);
    exception_write("Interrupt: ", 2, 0, 0x0F);
    exception_write_dec(regs->int_no, 2, 11, 0x0E);
    exception_write("Error code: ", 3, 0, 0x0F);
    exception_write_dec(regs->err_code, 3, 12, 0x0E);

    // Freeze the CPU because a real exception is a fatal error
    __asm__ volatile ("cli; hlt");
    while (1);
}

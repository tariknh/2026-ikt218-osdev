#include <kernel/pit.h>
#include <kernel/io.h>
#include <kernel/idt.h>
#include <kernel/terminal.h>

static volatile uint32_t pit_ticks = 0;

void pit_tick(void)
{
    pit_ticks++;
}

/* Compatibility with both common course skeleton styles. */
extern void register_interrupt_handler(uint8_t interrupt_number, isr_t handler) __attribute__((weak));
extern void irq_install_handler(int irq, isr_t handler) __attribute__((weak));

static void pit_callback(registers_t *regs)
{
    (void)regs;
    pit_tick();
}

uint32_t pit_get_ticks(void)
{
    return pit_ticks;
}

void init_pit(void)
{
    pit_ticks = 0;

    uint16_t divisor = (uint16_t)PIT_DIVIDER;
    outb(PIT_CMD_PORT, 0x36);                       /* channel 0, lobyte/hibyte, mode 3 */
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));

    if (register_interrupt_handler) {
        register_interrupt_handler(32, pit_callback); /* IRQ0 after PIC remap */
    } else if (irq_install_handler) {
        irq_install_handler(0, pit_callback);         /* older skeleton API */
    }

    /* Unmask IRQ0 on the master PIC. */
    uint8_t mask = inb(PIC1_DATA_PORT);
    outb(PIC1_DATA_PORT, mask & (uint8_t)~0x01u);

    printf("[PIT] Initialized at %d Hz.\n", TARGET_FREQUENCY);
}

void sleep_busy(uint32_t milliseconds)
{
    uint32_t start = pit_get_ticks();
    uint32_t ticks_to_wait = milliseconds * TICKS_PER_MS;
    while ((pit_get_ticks() - start) < ticks_to_wait) {
        /* busy wait */
    }
}

void sleep_interrupt(uint32_t milliseconds)
{
    uint32_t end_tick = pit_get_ticks() + (milliseconds * TICKS_PER_MS);
    while ((int32_t)(end_tick - pit_get_ticks()) > 0) {
        __asm__ volatile ("sti; hlt");
    }
}

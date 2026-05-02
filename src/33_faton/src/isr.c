#include <isr.h>
#include <monitor.h>
#include <libc/stdint.h>

#define PIC1_CMD 0x20
#define PIC2_CMD 0xA0
#define PIC_EOI  0x20

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "Nd"(port));
}

typedef struct {
    isr_t handler;
    void* data;
} int_handler_t;

static int_handler_t int_handlers[256];

void register_interrupt_handler(uint8_t n, isr_t handler, void* context) {
    int_handlers[n].handler = handler;
    int_handlers[n].data    = context;
}

void isr_handler(registers_t regs) {
    uint8_t int_no = regs.int_no & 0xFF;

    if (int_no >= 32) {
        if (int_no >= 40) {
            outb(PIC2_CMD, PIC_EOI);
        }
        outb(PIC1_CMD, PIC_EOI);
    }

    int_handler_t intrpt = int_handlers[int_no];
    if (intrpt.handler != 0) {
        intrpt.handler(&regs, intrpt.data);
    } else {
        printf("Unhandled interrupt: %d\n", int_no);
    }
}
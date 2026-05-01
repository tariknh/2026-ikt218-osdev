#include "pic.h"
#include "ports.h"
#include <libc/stdint.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

#define PIC_EOI 0x20

#define ICW1_INIT 0x10
#define ICW1_ICW4 0x01
#define ICW4_8086 0x01

void pic_remap(void)
{
    uint8_t master_mask = port_byte_in(PIC1_DATA);
    uint8_t slave_mask = port_byte_in(PIC2_DATA);

    port_byte_out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    port_byte_out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);

    port_byte_out(PIC1_DATA, 0x20);
    port_byte_out(PIC2_DATA, 0x28);

    port_byte_out(PIC1_DATA, 0x04);
    port_byte_out(PIC2_DATA, 0x02);

    port_byte_out(PIC1_DATA, ICW4_8086);
    port_byte_out(PIC2_DATA, ICW4_8086);

    port_byte_out(PIC1_DATA, master_mask);
    port_byte_out(PIC2_DATA, slave_mask);
}

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8) {
        port_byte_out(PIC2_COMMAND, PIC_EOI);
    }

    port_byte_out(PIC1_COMMAND, PIC_EOI);
}

void pic_mask_all_except_keyboard(void)
{
    port_byte_out(PIC1_DATA, 0xFD);
    port_byte_out(PIC2_DATA, 0xFF);
}

void pic_mask_all_except_timer_and_keyboard(void)
{
    port_byte_out(PIC1_DATA, 0xFC);
    port_byte_out(PIC2_DATA, 0xFF);
}

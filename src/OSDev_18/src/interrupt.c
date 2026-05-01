#include <kernel/interrupt.h>
#include <kernel/terminal.h>
#include <kernel/io.h>

#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1
#define PIC_EOI      0x20

static InterruptHandler interruptHandlers[256];

const char* isrMessages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void PicSendEoi(uint8_t irqNum) {
    if (irqNum >= 8) {
        OutPortByte(PIC2_COMMAND, PIC_EOI);  // Slave
    }
    OutPortByte(PIC1_COMMAND, PIC_EOI); // Master
}

void PicRemap(void) {
    OutPortByte(PIC1_COMMAND, 0x11); // sends init command to Master
    OutPortByte(PIC2_COMMAND, 0x11); // sends init command to Slave

    OutPortByte(PIC1_DATA, 0x20); // sets Master vector offset to 32
    OutPortByte(PIC2_DATA, 0x28); // sets Slave vector offset to 40

    OutPortByte(PIC1_DATA, 0x04); // Tells Master that Slave is available on IRQ2
    OutPortByte(PIC2_DATA, 0x02); // Tells Slave that it is connected to Master's IRQ2

    OutPortByte(PIC1_DATA, 0x01); // Set Master to use 8086 mode
    OutPortByte(PIC2_DATA, 0x01); // set Slave to use 8086 mode

    OutPortByte(PIC1_DATA, 0x00); // Unmask all Master IRQs
    OutPortByte(PIC2_DATA, 0x00); // Unmask all Slave IRQs
}

void IsrHandler(struct Registers* registers) {
    TerminalWriteString("\n=== INTERRUPT RECEIVED ===\n");

    TerminalWriteString("Interrupt Number: ");
    TerminalWriteUInt(registers->int_no);
    TerminalWriteString("\n");

    TerminalWriteString("Message: ");
    if (registers->int_no < 32) {
        TerminalWriteString(isrMessages[registers->int_no]);
    } else {
        TerminalWriteString("Unknown Interrupt");
    }
    TerminalWriteString("\n");

    TerminalWriteString("Error Code: ");
    TerminalWriteUInt(registers->err_code);
    TerminalWriteString("\n");

    TerminalWriteString("==========================\n");

    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}

void RegisterInterruptHandler(uint8_t iNum, InterruptHandler handler) {
    interruptHandlers[iNum] = handler;
}

void IrqHandler(struct Registers* registers) {
    uint8_t irqNum = (uint8_t)(registers->int_no - 32);

    if (interruptHandlers[registers->int_no] != 0) {
        interruptHandlers[registers->int_no](registers);
    } else if (irqNum != 0) {
        TerminalWriteString("IRQ triggered: ");
        TerminalWriteUInt(irqNum);
        TerminalWriteString("\n");
    }

    PicSendEoi(irqNum);
}

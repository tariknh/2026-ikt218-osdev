#include "irq.h"
#include "idt.h"
#include "terminal.h"
#include "libc/stdint.h"

/*
 * I/O-port hjelpefunksjoner.
 * outb sender en byte til en I/O-port, inb leser en byte.
 * Brukes for å snakke med PIC-en og andre hardware-kontrollere.
 */
static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Liten forsinkelse for PIC-kommandoer (noen eldre PIC-er trenger dette) */
static inline void io_wait(void)
{
    outb(0x80, 0);
}

/*
 * PIC-porter:
 *   Master PIC: kommando = 0x20, data = 0x21
 *   Slave PIC:  kommando = 0xA0, data = 0xA1
 */
#define PIC1_CMD    0x20
#define PIC1_DATA   0x21
#define PIC2_CMD    0xA0
#define PIC2_DATA   0xA1

/* End of Interrupt - signalet vi sender til PIC-en etter hvert interrupt */
#define PIC_EOI     0x20

/*
 * Tabell med custom handlere for hver IRQ.
 * Starter som NULL, og kan settes med irq_register_handler().
 * F.eks. tastatur-driveren registrerer seg på IRQ 1.
 */
static irq_handler_t irq_handlers[16] = { 0 };

/* Assembly-stubbene (definert i irq.asm) */
extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

/*
 * Registrerer en handler-funksjon for en bestemt IRQ.
 * Når den IRQ-en trigges, kalles denne funksjonen.
 */
void irq_register_handler(int irq, irq_handler_t handler)
{
    if (irq >= 0 && irq < 16) {
        irq_handlers[irq] = handler;
    }
}

/*
 * Remapper PIC-en (8259).
 *
 * Uten remapping sender PIC-en IRQ 0-7 som interrupt 8-15,
 * som kolliderer med CPU-exceptions (f.eks. double fault = int 8).
 * Etter remapping sender den IRQ 0-15 som interrupt 32-47.
 *
 * Dette er standard ICW (Initialization Command Word) sekvens:
 *   ICW1: Start initialisering (0x11 = med ICW4)
 *   ICW2: Sett offset (hvor IRQ-ene starter i IDT)
 *   ICW3: Fortell master/slave hvordan de er koblet
 *   ICW4: Sett modus (0x01 = 8086 modus)
 */
static void pic_remap(void)
{
    /* Lagre nåværende masker (hvilke IRQ-er som er aktivert) */
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    /* ICW1: Start initialisering av begge PIC-ene */
    outb(PIC1_CMD, 0x11);
    io_wait();
    outb(PIC2_CMD, 0x11);
    io_wait();

    /* ICW2: Sett interrupt-offset */
    outb(PIC1_DATA, 0x20);    /* Master: IRQ 0-7  -> interrupt 32-39 */
    io_wait();
    outb(PIC2_DATA, 0x28);    /* Slave:  IRQ 8-15 -> interrupt 40-47 */
    io_wait();

    /* ICW3: Fortell PIC-ene om kaskade-oppsettet */
    outb(PIC1_DATA, 0x04);    /* Master: slave henger på IRQ 2 (bit 2) */
    io_wait();
    outb(PIC2_DATA, 0x02);    /* Slave: jeg er koblet til IRQ 2 */
    io_wait();

    /* ICW4: Sett 8086-modus */
    outb(PIC1_DATA, 0x01);
    io_wait();
    outb(PIC2_DATA, 0x01);
    io_wait();

    /* Gjenopprett maskene vi lagret i starten */
    outb(PIC1_DATA, 0x00);
    outb(PIC2_DATA, 0x00);
}

/*
 * Setter opp IRQ-støtte:
 *   1. Remapper PIC-en så IRQ-ene ikke kolliderer med exceptions
 *   2. Registrerer alle 16 IRQ-stubbene i IDT-en
 *   3. Skrur på interrupts med sti-instruksjonen
 */
void irq_init(void)
{
    pic_remap();

    idt_set_entry(32, (uint32_t)irq0,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(33, (uint32_t)irq1,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(34, (uint32_t)irq2,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(35, (uint32_t)irq3,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(36, (uint32_t)irq4,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(37, (uint32_t)irq5,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(38, (uint32_t)irq6,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(39, (uint32_t)irq7,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(40, (uint32_t)irq8,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(41, (uint32_t)irq9,  0x08, IDT_FLAG_KERNEL);
    idt_set_entry(42, (uint32_t)irq10, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(43, (uint32_t)irq11, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(44, (uint32_t)irq12, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(45, (uint32_t)irq13, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(46, (uint32_t)irq14, 0x08, IDT_FLAG_KERNEL);
    idt_set_entry(47, (uint32_t)irq15, 0x08, IDT_FLAG_KERNEL);

    /* Nå er alt klart - skru på interrupts! */
    asm volatile("sti");
}

/*
 * Felles handler for alle hardware IRQ-er.
 *
 * Gjør tre ting:
 *   1. Sjekker om noen har registrert en custom handler for denne IRQ-en
 *   2. Kaller handleren hvis den finnes (f.eks. tastatur-driveren)
 *   3. Sender End of Interrupt (EOI) til PIC-en
 *
 * EOI er viktig! Uten den tror PIC-en at vi fortsatt jobber med
 * forrige interrupt og sender ikke flere. Slave PIC (IRQ 8-15)
 * trenger EOI til BEGGE PIC-ene.
 */
void irq_handler(struct isr_frame* frame)
{
    /* Finn hvilken IRQ dette var (interrupt 32 = IRQ 0, osv.) */
    int irq = frame->int_no - IRQ_OFFSET;

    /* Kall custom handler hvis den er registrert */
    if (irq >= 0 && irq < 16 && irq_handlers[irq]) {
        irq_handlers[irq](frame);
    }

    /* Send EOI til slave PIC hvis IRQ-en kom derfra (IRQ 8-15) */
    if (irq >= 8) {
        outb(PIC2_CMD, PIC_EOI);
    }

    /* Send alltid EOI til master PIC */
    outb(PIC1_CMD, PIC_EOI);
}
#include "idt.h"
#include "libc/string.h"

/*
 * Her setter vi opp Interrupt Descriptor Table (IDT).
 *
 * IDT-en forteller CPU-en hva den skal gjøre når et interrupt skjer.
 * Det finnes 256 mulige interrupts:
 *   - 0 til 31:   CPU-exceptions (f.eks. division by zero, page fault)
 *   - 32 til 47:  Hardware IRQ-er (tastatur, timer, osv.)
 *   - 48 til 255: Kan brukes til software-interrupts
 *
 * Vi fyller tabellen med nuller først, så registrerer vi handlere
 * for de interruptene vi faktisk trenger i isr_init() og irq_init().
 */

/* Selve IDT-tabellen med alle 256 entries */
static struct idt_entry idt[IDT_ENTRY_COUNT];

/* Pekeren som lidt-instruksjonen bruker */
static struct idt_ptr ip;

/* Assembly-funksjon som kjører lidt (definert i idt.asm) */
extern void idt_flush(struct idt_ptr* ptr);

/*
 * Setter opp én entry i IDT-en.
 * Base-adressen splittes i to deler fordi x86 er x86...
 */
void idt_set_entry(int num, uint32_t base, uint16_t selector, uint8_t flags)
{
    idt[num].base_low  = (uint16_t)(base & 0xFFFF);
    idt[num].base_high = (uint16_t)((base >> 16) & 0xFFFF);
    idt[num].selector  = selector;
    idt[num].zero      = 0;
    idt[num].flags     = flags;
}

/*
 * Nullstiller hele IDT-en og laster den inn i CPU-en.
 * De faktiske handler-adressene settes opp etterpå av
 * isr_init() og irq_init().
 */
void idt_init(void)
{
    ip.limit = (uint16_t)(sizeof(struct idt_entry) * IDT_ENTRY_COUNT - 1);
    ip.base  = (uint32_t)&idt;

    /* Nullstill alt først, så har ubrukte interrupts ingen handler */
    memset(&idt, 0, sizeof(struct idt_entry) * IDT_ENTRY_COUNT);

    /* Last IDT-en inn i CPU-en */
    idt_flush(&ip);
}
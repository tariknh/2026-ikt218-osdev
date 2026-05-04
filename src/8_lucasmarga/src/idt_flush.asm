; idt_flush.asm - load IDT using lidt

BITS 32
GLOBAL load_idt

load_idt:
    ; argument: pointer to idt_ptr at [esp + 4]
    mov eax, [esp + 4]

    ; load IDT
    lidt [eax]

    ret
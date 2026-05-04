; idt.asm - load the IDT
;
; The C function idt_install() builds the IDT in memory, then calls this
; routine with a pointer to a 6-byte (limit + base) descriptor.

global idt_flush

section .text
bits 32

idt_flush:
    mov     eax, [esp + 4]     ; argument: address of struct idt_ptr
    lidt    [eax]
    ret

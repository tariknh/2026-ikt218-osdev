; idt.asm - Laster Interrupt Descriptor Table inn i CPU-en
;
; Brukes fra C: void idt_flush(struct idt_ptr* ptr);
;
; lidt gjør omtrent det samme som lgdt - den forteller CPU-en
; hvor tabellen ligger i minnet og hvor stor den er.

global idt_flush

section .text
bits 32

idt_flush:
    mov eax, [esp+4]    ; Hent pekeren til idt_ptr fra stacken
    lidt [eax]          ; Last IDT-registeret
    ret
; idt_load.asm
; extern void idt_load(idtr_t_64*);

;global idt_load

section .text
idt_load:
    ; RDI holds pointer to idtr_t (SysV AMD64 ABI)
    lidt (%rdi)
    ret

global idt_load

; Loads IDT.
;
; C prototype:
;   void idt_load(uint32_t idt_ptr_address);
;
; Stack layout:
;   [esp + 4] = address of struct idt_ptr

idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

BITS 32
GLOBAL idt_load

; void idt_load(idt_ptr_t* ptr)
; GCC will pass ptr on stack, but we'll immediately load it safely
idt_load:
    mov eax, [esp + 4]
    lidt [eax]
    ret

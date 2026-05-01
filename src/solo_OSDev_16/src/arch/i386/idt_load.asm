global idt_load

idt_load:
    mov eax, [esp + 4]      ; Get IDT pointer
    lidt [eax]              ; Load IDT
    ret                     ; Return
global idt_load

idt_load:
    mov eax, [esp + 4] ; Load the address of the IDT descriptor into eax
    lidt [eax]           ; Load the IDT using the lidt instruction
    ret                   ; Return from the function
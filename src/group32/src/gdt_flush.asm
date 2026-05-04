global gdt_flush

gdt_flush:
    mov eax, [esp + 4] ; Get the GDT pointer from the stack
    lgdt [eax] ; Load the new GDT

    mov ax, 0x10 ; Load the data segment selector
    mov ds, ax ; Update DS segment
    mov es, ax ; Update ES segment
    mov fs, ax ; Update FS segment
    mov gs, ax ; Update GS segment
    mov ss, ax ; Update SS segment

    jmp 0x08:.flush ; Far jump to reload the code segment
.flush:
    ret ; Return to the C function
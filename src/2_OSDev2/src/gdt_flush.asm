global gdt_flush

gdt_flush:
    mov eax, [esp + 4] ; Load the address of the GDT into eax
    lgdt [eax]          ; Load the GDT using the lgdt instruction

    mov ax, 0x10       ; Load the data segment selector (0x10) into ax (index 2 << 3)
    mov ds, ax         ; Update the data segment register
    mov es, ax         ; Update the extra segment register
    mov fs, ax         ; Update the fs segment register
    mov gs, ax         ; Update the gs segment register
    mov ss, ax         ; Update the stack segment register

    jmp 0x08:.flush_cs ; Jump to the code segment selector (0x08) to flush the instruction pipeline
.flush_cs:
    ret
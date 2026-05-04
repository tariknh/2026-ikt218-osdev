global gdt_flush     ;make function accessible from C

gdt_flush:
    mov eax, [esp+4] ;get argument passed from C
    lgdt [eax]       ;load GDT into CPU

.reloadSegments:
    mov ax, 0x10     ;data segment selector
    mov ds, ax       ;reload all data segment registers
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.done   ;jump to next line to update CS (code segment) register (mov doesn't work for CS)
.done:    
    ret
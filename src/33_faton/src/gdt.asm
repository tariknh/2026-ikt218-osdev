global gdt_flush

gdt_flush:
    mov eax, [esp+4]    ; Hent GDT pointer fra stack
    lgdt [eax]          ; Last GDT

    mov ax, 0x10        ; Data segment selector (entry 2)
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:.flush     ; Far jump til code segment (entry 1)

.flush:
    ret
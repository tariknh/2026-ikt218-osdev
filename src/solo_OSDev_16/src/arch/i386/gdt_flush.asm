global gdt_flush

gdt_flush:
    mov eax, [esp + 4]      ; Get the address of the gdt_ptr (esp = 32-bit stack pointer)
    lgdt [eax]              ; Load the new GDT

    mov ax, 0x10            ; Data segment selector
    mov ds, ax             
    mov es, ax              
    mov fs, ax              
    mov gs, ax              
    mov ss, ax              

    jmp 0x08:.flush         ; Far jump to reload the code segment register
.flush:
    ret


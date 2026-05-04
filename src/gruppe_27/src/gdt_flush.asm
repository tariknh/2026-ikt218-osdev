[BITS 32]
global gdt_flush

section .text

gdt_flush:
    ; The pointer to the GDT is passed as the first argument on the stack
    mov eax, [esp + 4]  
    lgdt [eax]          ; Load the new GDT pointer

    ; Reload Data Segment Registers
    ; 0x10 is the offset to our Data Segment (the 3rd entry in GDT)
    mov ax, 0x10      
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Reload Code Segment (CS)
    ; We must use a 'far jump' to do this. 
    ; 0x08 is the offset to our Code Segment (the 2nd entry in GDT)
    jmp 0x08:.flush_cs

.flush_cs:
    ret
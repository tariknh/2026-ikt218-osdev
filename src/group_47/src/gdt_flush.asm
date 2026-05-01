; In this file, we transition from real mode to protected mode by loading the GDT (Global Descriptor Table) and updating the segment registers accordingly. 
;This is done in the gdt_flush function, which is called from C code after setting up the GDT entries.
[bits 32]
global gdt_flush

gdt_flush:
    mov eax, [esp + 4]  ; Retrieving the parameter from C code (pointer to gdt_ptr)
    lgdt [eax]          ; Loading the GDT pointer into the CPU's GDTR register

    ; Updateing all segment registers to point to the new Data segment.
    ; In our init_gdt(), the Data segment is entry #2, which has offset 0x10 (2 * 8 bytes).
    mov ax, 0x10      
    mov ds, ax ; Data Segment
    mov es, ax ; Extra Segment
    mov fs, ax ; FS Segment
    mov gs, ax ; GS Segment
    mov ss, ax ; Stack Segment

    ; At the end does the "Far Jump" to update the Code Segment 
    ; The Code-segment is nr 1, and have offset 0x08 (1 * 8 bytes).
    jmp 0x08:.flush_cs

.flush_cs:
    ret  ; Returning to the caller in C code after the flush is complete
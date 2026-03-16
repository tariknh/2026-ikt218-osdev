; gdt.asm - Load the GDT and reload all segment registers
;
; Exported symbol used from C:
;   void gdt_flush(struct gdt_ptr* ptr);
;
; The C calling convention (cdecl) passes the first argument on the stack
; at [esp+4] after the return address is pushed by the call instruction.
;
; Steps performed:
;   1. Load the GDT pointer with lgdt.
;   2. Reload DS, ES, FS, GS, SS with the kernel data selector (0x10).
;   3. Far-jump to the kernel code selector (0x08) to reload CS and
;      flush the CPU instruction pipeline.

global gdt_flush

section .text
bits 32

gdt_flush:
    mov eax, [esp+4]        ; Load argument: pointer to struct gdt_ptr
    lgdt [eax]              ; Tell the CPU where the GDT is

    ; Reload data-segment registers with the kernel data selector.
    ; Selector 0x10 = GDT entry 2 (index 2 * 8 = 16 = 0x10), RPL=0, TI=0.
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Reload CS (code segment) with the kernel code selector via a far jump.
    ; Selector 0x08 = GDT entry 1 (index 1 * 8 = 8 = 0x08), RPL=0, TI=0.
    ; A far jump changes both EIP and CS atomically, flushing the pipeline.
    jmp 0x08:.flush

.flush:
    ret

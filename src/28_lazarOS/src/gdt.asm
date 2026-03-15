; gdt.asm
;
; Low-level GDT routines called from C.

global gdt_load
global gdt_reload_segments

section .text
bits 32

; void gdt_load(gdt_descriptor_t *descriptor)
gdt_load:
	mov eax, [esp + 4]
	lgdt [eax]
	ret

; void gdt_reload_segments(void)
gdt_reload_segments:
	jmp 0x08:.reload_cs

.reload_cs:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	ret


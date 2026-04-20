
[bits 32]

global gdt_set_gdt
global gdt_reload_segments

; Loads a GDT
; ESP: ret address 4 bytes bit
; ESP + 4: gdtr_ptr 4 bytes
gdt_set_gdt:
   ; Pointer to struct is at [ESP + 4]
   MOV   EAX, [ESP + 4]

   LGDT  [EAX]
   RET

; Reloads segments after a gdt update
gdt_reload_segments:
   ; Reload CS register containing code selector:
   JMP   0x08:.reload_CS ; Kernel Code Segment: 0x08

; Reloads the code segments
.reload_CS:
   ; Reload data segment registers:
   MOV   AX, 0x10 ; Kernel Data segment: 0x10
   MOV   DS, AX
   MOV   ES, AX
   MOV   FS, AX
   MOV   GS, AX
   MOV   SS, AX
   RET

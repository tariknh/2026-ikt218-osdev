#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/irq.h"
#include "../include/isr.h"
#include "../include/kernel/memory.h"
#include "../include/kernel/pit.h"
#include "../include/keyboard.h"
#include "../include/libc/stdbool.h"
#include "../include/libc/stddef.h"
#include "../include/libc/stdint.h"
#include "../include/libc/stdio.h"
#include "../include/matrix.h"
#include "../include/menu.h"
#include "../include/multiboot2.h"
#include "../include/piano.h"
#include "../include/program.h"
#include "lib/song/song.h"

extern uint32_t end; // The memory address of where the kernel ends

int kernel_main();

int main(uint32_t magic, struct multiboot_info *mb_info_addr) {
  // Requirements from assignment 2
  gdt_init();
  // Requirements from assignment 3
  idt_init();
  isr_init();
  irq_init();
  keyboard_init();
  // Requirements from assignment 4
  init_kernel_memory(&end);
  init_paging();
  print_memory_layout();
  init_pit();

  // Enable interrupts after we disabled them in _start in multiboot2.h
  asm volatile("sti");

  int reload = 250;
  int counter = reload;

  printf("Hello World!\n");
  asm volatile("int $0x00");
  asm volatile("int $0x01");
  asm volatile("int $0x02");
  printf("Memory information printed above. To display the menu press: %d \n",
         PROGRAM_MENU);
  // Main kernel loop
  while (true) {
    int entry = kb_dequeue(&kb);
    if (entry != -1) {
      keyboard_handler(entry);
    }
    if (active_program == PROGRAM_PIANO && piano_played_key) {
      // Uses a counter and reload value to make the matrix
      // update happen less frequently.
      if (--counter == 0) {
        counter = reload;
        matrix_rain_frame();
      }
    }
    sleep_interrupt(1);
  }

  return kernel_main();
}

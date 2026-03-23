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
#include "../include/multiboot2.h"
#include "lib/song/song.h"

extern uint32_t
    end; // Defined in arch/i386/linker.ld — marks the end of the kernel image

int kernel_main();

int main(uint32_t magic, struct multiboot_info *mb_info_addr) {
  gdt_init();
  idt_init();
  isr_init();
  irq_init();
  keyboard_init();

  // Initialize kernel heap starting just past the kernel image.
  init_kernel_memory(&end);

  // Set up basic identity-mapped paging.
  init_paging();

  // Display heap/paging layout.
  print_memory_layout();

  // Initialise the PIT at 1000 Hz (1 tick per ms).
  init_pit();

  // Enable interrupts — the CPU starts with interrupts disabled (cli in
  // _start). Only safe to do after GDT, IDT, ISRs, IRQs and PIT are all set up.
  asm volatile("sti");

  printf("hello world\n");

  // Quick malloc smoke-test
  void *some_memory = malloc(12345);
  void *memory2 = malloc(54321);
  void *memory3 = malloc(13331);

  // PIT sleep test
  // int counter = 0;
  // while (1) {
  //   printf("[%d]: Sleeping with busy-waiting (HIGH CPU).\n", counter);
  //   sleep_busy(1000);
  //   printf("[%d]: Slept using busy-waiting.\n", counter++);
  //
  //   printf("[%d]: Sleeping with interrupts (LOW CPU).\n", counter);
  //   sleep_interrupt(1000);
  //   printf("[%d]: Slept using interrupts.\n", counter++);
  // }

  SongPlayer *songPlayer = create_song_player();

  printf("attempting to play song");
  Song song_1 = {.notes = music_4,
                 .length = sizeof(music_1) / sizeof(music_1[0])};
  songPlayer->play_song(&song_1);
  printf("Finished playing song");

  // Call cpp kernel_main (defined in kernel.cpp)
  return kernel_main();
}

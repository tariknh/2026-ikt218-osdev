#include <drivers/input/keyboard.h>
#include <kernel/pit.h>
#include <kernel/util.h>
#include <stdint.h>
#include <stdio.h>

int cmd_timer_test(int argc, char** argv) {
  (void)argc;
  (void)(argv);
  printf("Timer test started. \n Press enter to run a cycle.  Press ESC or Ctrl+C to exit.\n");
  while (1) {
    decode_keyboard();
    uint8_t key = 0;
    pop_key(&key);

    if (key != 0) {
      if (key == 27 || key == 3) {
        printf("\nExited.\n");
        return 0;
      }

      static uint32_t counter = 0;
      printf("[%d]: Sleeping for 1sec with busy-waiting (HIGH CPU).\n", counter);
      uint64_t start = rdtsc();
      sleep_busy(1000);
      printf("[%d]: Slept using busy-waiting: %llu\n", counter++, rdtsc() - start);

      printf("[%d]: Sleeping for 1sec with interrupts (LOW CPU).\n", counter);
      start = rdtsc();
      sleep_interrupt(1000);
      printf("[%d]: Slept using interrupts: %llu \n", counter++, rdtsc() - start);
    }
  }
  return 0;
}

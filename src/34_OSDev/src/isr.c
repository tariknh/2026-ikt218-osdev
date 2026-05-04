#include "isr.h"
#include "terminal.h"

void isr_handler(uint32_t interrupt_number) {
    printf("Interrupt triggered: %d\n", interrupt_number);
    while (1) {} //stop CPU here cause returning from an interrupt handler could make it trigger again
}
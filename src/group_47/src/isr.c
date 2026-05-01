#include "isr.h"
#include "terminal.h"  

// This file contains the implementation of the Interrupt Service Routines (ISRs), which are responsible for handling 
//CPU exceptions and software interrupts.
void isr_handler(registers_t *regs) {
    // Function in assembly to handle CPU exceptions (ISRs).
    switch(regs->int_no) {
        case 0:
            printf("Divide by Zero exception triggered!\n");
            break;
        case 1:
            printf("Debug exception triggered!\n");
            break;
        case 2:
            printf("Non-maskable interrupt (NMI) triggered!\n");
            break;
        default:
            // Default case for any unhandled CPU exceptions 
            if (regs->int_no < 32) {
                printf("Unknown CPU Exception: ");
                print_int(regs->int_no);
                printf("\n");
            }
            break;
    }
    
    // Halt for any CPU exception 
    if (regs->int_no < 32) {
        printf("System halted.\n");
        for(;;);  // Infinite loop
    }
}
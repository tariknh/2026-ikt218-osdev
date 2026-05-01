#ifndef ISR_H
#define ISR_H

// Assembly ISR stubs
extern void isr0(void);         // Divide by zero exception, vector 0
extern void isr3(void);         // Breakpoint exception, vector 3
extern void isr14(void);        // Page fault exception, vector 14

// C handler functions
void isr0_handler(void);
void isr3_handler(void);
void isr14_handler(void);

#endif
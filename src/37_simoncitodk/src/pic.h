#ifndef PIC_H
#define PIC_H

#include <libc/stdint.h>

void pic_remap(void);
void pic_send_eoi(uint8_t irq);
void pic_mask_all_except_keyboard(void);
void pic_mask_all_except_timer_and_keyboard(void);

#endif

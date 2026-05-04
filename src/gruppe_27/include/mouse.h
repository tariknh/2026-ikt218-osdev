#ifndef MOUSE_H
#define MOUSE_H
#include <stdint.h>

void mouse_install();
void mouse_set_drawing_mode(int active);
int  mouse_right_clicked();
int  mouse_get_x();
int  mouse_get_y();
int  mouse_left_held();
extern volatile int drawing_mode_active;

#endif
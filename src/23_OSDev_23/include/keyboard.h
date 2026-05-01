#ifndef KEYBOARD_H
#define KEYBOARD_H

/* Install the keyboard handler on IRQ1 */
void keyboard_install(void);
char keyboard_getchar(void);

#endif /* KEYBOARD_H */

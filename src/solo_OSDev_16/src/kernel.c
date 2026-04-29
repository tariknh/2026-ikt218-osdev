#include "gdt.h" 
#include "terminal.h"


void kmain(void) {
gdt_initialize();
terminal_initialize();
terminal_write("Hello World");

while(1){}
}
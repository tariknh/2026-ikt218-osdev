#include <libc/stdint.h>
    extern void terminal_write(const char *str);

__attribute__((noreturn))
void exception_handler(uint32_t vector) {
    
    if(vector == 0){
        terminal_write("INT 0\n");
    } 
    if(vector == 3){
        terminal_write("INT 3\n");
    } 
    if(vector == 6){
        terminal_write("INT 6\n");
    } 
    

}

#include "libc/stdint.h"
#include "libc/stddef.h"
#include "libc/stdbool.h"
#include <multiboot2.h>
#include "interrupts/interrupts.h"

/**
 * \enum Gives acces to standard VGA text mode colors.
 * 
 * \note In MDA mode you can set underlines instead of color. [How to switch to MDA mode?]
 * \see https://en.wikipedia.org/wiki/VGA_text_mode#Underline
 * 
 * \note Attribiute bit 7 can be used to make the text blink.
 * In this case setting the background color to an high intensity color (>7)
 * will make the text blink with the corresponding low intensity background color.
 * \see https://en.wikipedia.org/wiki/VGA_text_mode#Text_buffer
 * 
 * \note VGA text mode supports multiple fonts. And two of them can be used at the same time.
 * But in that case attribute bit 3 will change the font for the corresponding cell.
 * So high intensity foreground colors will change the font if two are loaded.
 * This should not be a problem if both font pointers point to the same font.
 * On some VGA cards both the color and font will be changed at the same time.
 * If you want two fonts with the same colors you can manually change the color palette.
 * \see https://en.wikipedia.org/wiki/VGA_text_mode#Fonts
 * 
 * \note The color palette can be changed using VGA commands. [How?]
 * \see https://wiki.osdev.org/Text_UI#Colours
 */
enum standard_vga_text_color{
    vga_black = 0,
    vga_blue = 1,
    vga_green = 2,
    vga_cyan = 3,
    vga_red = 4,
    vga_magenta = 5,
    vga_brown = 6,
    vga_light_grey = 7,
    vga_dark_gray = 8,
    vga_light_blue = 9,
    vga_light_green = 10,
    vga_light_cyan = 11,
    vga_light_red = 12,
    vga_light_magenta = 13,
    vga_yellow = 14,
    vga_white = 15
};

/** \brief Puts together two values from standard_vga_text_color into a single uint8_t 
 *  \todo Switch to an oop system that handles underlaines and multiple fonts
*/
uint8_t VgaColor(enum standard_vga_text_color bg, enum standard_vga_text_color fg){
    return bg<<4|fg;
}

/// @brief Copies every element of one array to every even position in the output array until it encounters a 0.
///
/// input_array = [a,b,0] and input_array = [c,d,e,f] wil make output_array become [a,d,b,f]
/// This function is mostly to help with VGA text mode.
/// @param input_array A pointer to the start of the zero-terminated input array.
/// @param output_array A pointer to the start of the output array
/// \warning The input array MUST be zero-terminated.
/// \note Does not change the odd values in the output array.
/// \deprecated Use VgaTextModeInterface.Print instead
void copyZeroTerminatedCharArrayToEvenPositionsInCharArray(char* input_array, char* output_array){
    while ( *input_array != 0 ){
        *(output_array++) = *(input_array++);
        output_array++;
    }
}

#include "VgaTextModeInterface.c"

struct multiboot_info {
    uint32_t size;
    uint32_t reserved;
    struct multiboot_tag *first;
};



int main(uint32_t magic, struct multiboot_info* mb_info_addr) {
    gdt_init();
    // char a[]= "Hello World!!";
    // char* vga_text = (char *) 0xb8000;
    // copyZeroTerminatedCharArrayToEvenPositionsInCharArray((char*)&a, vga_text);

    struct VgaTextModeInterface screen = NewVgaTextModeInterface();
    screen.Print(&screen, "GDT loaded successfully!", VgaColor(vga_cyan, vga_black));

    // GDT Test:
    uint16_t cs, ds, ss;
    __asm__ __volatile__("mov %%cs, %0" : "=r"(cs));
    __asm__ __volatile__("mov %%ds, %0" : "=r"(ds));
    __asm__ __volatile__("mov %%ss, %0" : "=r"(ss));

    if (cs == 0x08 && ds == 0x10 && ss == 0x10) {
        screen.Print(&screen, "GDT OK", VgaColor(vga_light_green, vga_black));
    } else {
        screen.Print(&screen, "GDT BAD", VgaColor(vga_light_red, vga_black));
    }

    init_idt();
    screen.Print(&screen, "IDT is initilalized", VgaColor(vga_black, vga_white));

    

    // Test how the os handels overflow:
    // while(1){screen.Print(&screen, "aaaaaaaaaaaaaaaaaaaaaa", VgaColor(vga_white, vga_black));}
    
    //Dont let the OS insta-reboot
    while(1){}
    return 0;
}
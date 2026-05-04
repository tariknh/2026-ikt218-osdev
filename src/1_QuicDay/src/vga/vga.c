#define VGA_TEXT_BUFFER 0xb8000u


void clear_text(unsigned char attrib) 
{
    volatile unsigned short *VideoMemory = (unsigned short *)VGA_TEXT_BUFFER;
    unsigned short blank = (attrib << 8) | ' ';
    for (int i = 0; i < 4000; i++)
        VideoMemory[i] = blank;
}

void print_string(const char *s, unsigned char attrib)
{
    volatile unsigned short *VideoMemory = (unsigned short *)VGA_TEXT_BUFFER;
    while (*s)
        *VideoMemory++ = (attrib << 8) | *s++;
}



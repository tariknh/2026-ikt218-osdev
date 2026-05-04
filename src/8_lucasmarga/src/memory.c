#include "memory.h"
#include "terminal.h"
#include "libc/stdint.h"

static uint32_t* page_directory = 0;   
static uint32_t page_dir_loc = 0;      
static uint32_t* last_page = 0;       


// Translates virtual memory to physical memory 
void paging_map_virtual_to_phys(uint32_t virt, uint32_t phys)
{
    uint16_t id = virt >> 22;        
    for(int i = 0; i < 1024; i++)   
    {
        last_page[i] = phys | 3;    
        phys += 4096;               
    }
    page_directory[id] = ((uint32_t)last_page) | 3;  
    last_page = (uint32_t *)(((uint32_t)last_page) + 4096); 
}

// Activates paging
void paging_enable()
{
    asm volatile("mov %%eax, %%cr3": :"a"(page_dir_loc)); 
    asm volatile("mov %cr0, %eax");        
    asm volatile("orl $0x80000000, %eax");  
    asm volatile("mov %eax, %cr0");         
}

// Sets up paging
void init_paging()
{
    terminal_print_string("Setting up paging\n");
    page_directory = (uint32_t*)0x400000;      
    page_dir_loc = (uint32_t)page_directory;  
    last_page = (uint32_t *)0x404000;         
    for(int i = 0; i < 1024; i++)             
    {
        page_directory[i] = 0 | 2;            
    }    
    paging_map_virtual_to_phys(0, 0);         
    paging_map_virtual_to_phys(0x400000, 0x400000); 
    paging_enable();                          
    terminal_print_string("Paging is now up and running!\n");
}

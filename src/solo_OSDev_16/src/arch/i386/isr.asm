; Make this assembly label available to other files
global isr0         
global isr3 
global isr14 

; Call external C handlers from the ISR stubs
extern isr0_handler
extern isr3_handler
extern isr14_handler

; Divide-by-zero exception handler
isr0:
    cli                     
    call isr0_handler       
    iret                    

; Breakpoint exception handler
isr3: 
    cli                     
    call isr3_handler       
    iret                     

; Page fault exception handler
isr14:
    cli                     
    call isr14_handler      
    iret                     
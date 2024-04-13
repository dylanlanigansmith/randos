#include "../drivers/video/console.h"
#include "stdlib.h"
#include "../drivers/port/port.h"
#include "idt/idt.h"
#include "idt/isr.h"
#include "pic/pic.h"
#include "timer/timer.h"
#include "../drivers/input/keyboard.h"
#include "../drivers/video/gfx.h"
#include "../drivers/video/vbe.h"

#include "../drivers/sound/sound.h"
#include "boot/multiboot2.h"
#include "boot/mb_header.h"

#include "sys/sysinfo.h"
#include "../drivers/serial/serial.h"

#include "mem/page.h"
#include "mem/heap.h"

#include "task/task.h"
#include "stdio/stdout.h"
extern unsigned long GDT_CODE_OFFSET;

int cpp_test(int, int);


uint64_t start_tick = 0;

bool first_call = False;

uint64_t last_tick = 0;
void task_drawtimer(){
    
    serial_print("task draw timer \n");
     gfx_clear_line(598, 24);
    for(;;){
        if((last_tick + 100) < tick  ){
             //   serial_printi("t=", tick);
                 gfx_clear_line(598, 24);
                last_tick = tick;
                gfx_print_pos(lltoa(tick, 10), v2(5, 600));
            }
    }
}
uint64_t last_tick2 = 0;
void task_draw_test(){
    
    serial_print("task draw test \n");
     gfx_clear_line(498, 24);
    for(;;){
        __asm__("hlt");
        if((last_tick2 + 100) < tick  ){

            if(stdout_dirty()){
                 gfx_clear_text();
                 __asm__ volatile ("cli");
                  gfx_print(get_stdout());
                  stdout_flush();
                  __asm__ volatile ("sti");
            }
             
         //   gfx_clear(gfx_state.clear_color);
           
             //   serial_printi("t=", tick);
              //  gfx_clear_line(498, 24);
               // last_tick2 = tick;
                //gfx_print_pos(lltoa(tick, 10), v2(5, 500));
            }
    }
}

void main(void *addr, void *magic)
{
    if ((uint64_t)magic != MULTIBOOT2_BOOTLOADER_MAGIC){ /* uh how? */ }
    //we do check this in parse_multiboot_header() but that does happen rather late in the start process 
    disable_interupts();
    PIC_init();
   
    init_idt();
    
    // register irq handlers
    keyboard_init();
    timer_init(PIT_RATE);

    
    serial_init(); //we can now debug and log! 
    debugf("multiboot2 addr = %lx magic = %lx \n",  (uintptr_t)addr, (uint64_t)magic);
    
    if(parse_multiboot_header(addr, (uint64_t)magic) == MB_HEADER_PARSE_ERROR){ //gets acpi, framebuffer, etc
        panic("multiboot parse fail");
    }
   
  
    _init_cpp(); //bc we cant get global constructors to work & by we i mean I
    __asm__("sti");
    serial_println("enabled interupts"); //should we do this after paging ??? 
   
    make_page_struct(); //this also initializes heap, maps frame buffer
    
    stdout_init();
    gfx_init(color_cyan);
    //we are so back
    
    tasking_init(&task_drawtimer); //first task should be on our main stack as its new kernel main 
    add_task("task_draw_test",task_draw_test);
    //we have gotten ourselves a system with two processes running
    // do a little dance or something 

    /*
        things to fix now
        - need physical memory management or at least a semblance of it
        - page mapping - just use kmalloc and figure out physical address
            - bonus points if it works in other emus again 
        - make stdout not fucked

        then we can jump 2 user mode
        - refactor when we add syscalls
            - take all this SHIT out of kernel
        - terminal
        - write some programs for _our operating system_
        -^ see prev.
    */

    ASSERT(gfx_has_init());
   // void* test = kmalloc(512);
  //  printf("trying virt to phys: %lx \n", virt_to_phys((uintptr_t)test) );
   // kfree(test);

    map_phys_addr(0xffffbaddad000000llu, sysinfo.rsdp->RsdtAddress, PAGE_SIZE ) ;

     printf("tasking time :( %lx %lx \n", (uintptr_t)task_drawtimer, (uintptr_t)task_draw_test);
    
   // task_draw_test();
   // start_first_task();

    size_t old_len = 0;
    char last_top = stdout_top();
    printf("old len = %li last top %c", old_len, last_top);

    uint64_t last_tick = 0;
    for(;;){
        __asm__("hlt");
        //solve YOUR OLDLEN struggles with one simple TRICK
            //let stdout mark itself as dirty 
        while (old_len != stdout.index ){ //|| last_top != stdout_top() //for backspace 
           
           // if(!stdout_update()) continue;
            gfx_clear_text();
         //   gfx_clear(gfx_state.clear_color);
            gfx_print(get_stdout());
            //for(int i = 0; i < stdout.index; ++i)
           //     gfx_putc(stdout.buffer[stdout.index]);
           
         //   gfx_putc(stdout_top());
           
            old_len = stdout.index;
            last_top = stdout_top();
            break;
             
          //  if(last_top) stdout_putchar('$');
        }
      ///  if((last_tick + 100) < tick  ){
             //   serial_printi("t=", tick);
         //       gfx_clear_line(598, 24);
        //        last_tick = tick;
       //         gfx_print_pos(lltoa(tick, 10), v2(5, 600));
         //   }
         
        
    }

    /*
    
    Scheduler:
        test: one process that reads a bunch of values and updates stdout

        task 1-5: changing values etc

        PIT runs for x interval, then switches task
    */
    for(;;){
        __asm__("hlt");
    }
    /*
        now what?

        kernel heap //in progress//
        at least get a malloc

            then we gotta figure out page api
            then we can do fork/tasks
            then jump to user mode 

            goal: game in USER MODE 
                also lets write game in cpp please

                 c++ ?
                 cross compiler ?
        user mode ?


        FILES = RAM DISK

    
    */


    // kernel ends, we can return to entry pt which hangs or just do it here for transparency while we develop
    __asm__("cli");
    for (;;)
    {
        __asm__("hlt");
    }
    __builtin_unreachable();
}


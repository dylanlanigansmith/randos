#include "syscall.h"

#include "../idt/isr.h"
#include "../task/task.h"
#include "../../drivers/video/gfx.h"
#include "../../drivers/port/port.h"
#include "../../drivers/input/keyboard.h"
#include "../timer/timer.h"
#include "sysinfo.h"
#include "../fs/initrd.h"

void* syscall_test(registers_t* regs){
    print("your sis called ;)");

    return 0;
}


void* sys_print(registers_t* regs){
    if(regs->rdi != 0){
        print((char*)regs->rdi);
    }

    return 0;
}
void* sys_putc(registers_t* regs){
    putc((char)(regs->rdi & 0xff) );
  //  debugf("put char %c %i \n",  (char)(regs->rdi & 0xff), (char)(regs->rdi & 0xff));
    return 0;
}
void* sys_clearscreen(registers_t* regs){

    color clr; clr.argb = (uint32_t)regs->rdi;
    if(regs->rdi < 255) clr = gfx_state.clear_color;
   gfx_clear(clr);
    
    return 0;
}
void* sys_setgfxmode(registers_t* regs){
    gfx_clear(color_black);

    if(gfx_state.mode == 0)
        gfx_state.mode = 1;
    else gfx_state.mode = 1;
    //TODO

    return sysinfo.fb.addr;
}


void* sys_togglegamemode(registers_t* regs){
    gfx_clear(color_black);

    if(gfx_state.mode == 0)
        gfx_state.mode = 1;
    else gfx_state.mode = 1;
    //TODO

    return sysinfo.fb.addr;
}



void* sys_exit(registers_t* regs){
    exit(regs->rdi);

    return 0; //never returns
}

void* sys_yield(registers_t* regs){
    yield();

    return 0; //never returns
}
void* sys_sleepms(registers_t* regs){
    if(regs->rdi == 0) return 1;
    task_sleep_ms(regs->rdi);

    return 0; //never returns
}


void* sys_getkeyhist(registers_t* regs){
    regs->rax = keys_getqueued();
    return regs->rax;
}

void* sys_getlastkey(registers_t* regs){
    regs->rax = keys_last_event();
    return regs->rax;
}

void* sys_iskeydown(registers_t* regs){
    uint8_t sc = (uint8_t)(regs->rax & 0xff) ;
    regs->rax = is_key_down(sc);
    return regs->rax & 0xff;
}

void* sys_getcurtick(registers_t* regs){
   return tick;
}

void* sys_getlastkeytick(registers_t* regs){
   return last_key_event;
}

void* sys_exec(registers_t* regs){
    int err = 1;
    if(regs->rdi != 0){
        err = exec_user_task((char*)regs->rdi, regs);
    }

    return err;
}


void* sys_shutdown(registers_t* regs){
    if(sysinfo.host_type == HOST_BOCHS){
         debugf("shutting down via BOCHS method\n");
        port_word_out(0xB004, 0x2000);
        
    }
    else if(sysinfo.host_type == HOST_QEMU){
         debugf("shutting down via QEMU method\n");
        port_word_out(0x604, 0x0 | 0x2000);
    }
    else if(sysinfo.host_type == HOST_VM_OTHER){ //probs vbox
        debugf("shutting down via VBOX method on a whim\n");
         port_word_out(0x4004, 0x3400);
    }
    else {
        debugf("shutting down via stupid method\n");
        __asm__ volatile ("cli; lidt [rax]; int 6; lgdt [rax]"); //should do the trick
    }

   return 0;
}

void* sys_getfsroot(registers_t* regs){ //does it deserve this tho

    initrd_demo();
   return initrd_root;
}


void* sys_requestheap(registers_t* regs){ //does it deserve this tho

   return initrd_root;
}

void* sys_brk(registers_t* regs){ 
    //AYO GIMME SOME MEMORY BRO
    size_t size = regs->rdi;

   void* r =  task_alloc_heap(size);
   
   
    return r;

}

void* sys_mmap(registers_t* regs){ 
    //AYO GIMME SOME MEMORY BRO
    const char* name = (char*)regs->rdi;

   void* r =  task_mmap_file(name);
   debug("mmaped that shit");
   
    return r;

}


typedef void* (*syscall_fn)(registers_t* regs);

#define SYSCALL_TOTAL 19 //this is stupid
static void* syscalls[] =
{
    &syscall_test, //0
    &sys_exit, 
    &sys_yield,
    &sys_sleepms,
    &sys_print, //4
    &sys_putc,
    &sys_clearscreen,
    &sys_setgfxmode,
    &sys_getlastkey, //8
    &sys_iskeydown,
    &sys_getcurtick,
    &sys_getlastkeytick, 
    &sys_shutdown, // 12
    &sys_exec,
    &sys_getfsroot,
    &sys_brk, //15
    &sys_mmap,
    &sys_getkeyhist,
    &sys_togglegamemode, //18
};

static_assert(SYSCALL_TOTAL == (sizeof(syscalls) / sizeof(void*)), "SYSCALL TOTAL NOT UPDATED");


static void handle_syscall(registers_t* regs){

    if(regs->rax >= SYSCALL_TOTAL) return;

    syscall_fn call = syscalls[regs->rax];

    //how do we want to handle args
    /*
        something like make macros for syscall wrappers

        parse args (rdi) into fmt
        idk
    */
    void* ret = call(regs);

    regs->rax = ret;



}

void syscall_init()
{
    register_interupt_handler(SYSCALL_INT, &handle_syscall);
}
#include "stdint.h"
#include "os.h"
#include "stdlib.h"


typedef struct {
    uintptr_t base;
    uintptr_t addr;
    size_t size;

} malloc_state_t;

malloc_state_t allstate = {0,0};  

static uintptr_t round_up_to_page(uintptr_t addr){
    if(addr % PAGE_SIZE == 0) return addr;
    return (addr + (PAGE_SIZE - (addr % PAGE_SIZE)));
}


void* brk(size_t sz){
    uintptr_t ret = 0;
     __asm__ volatile ( "mov rdi, %1; mov rax, 15; int 0x69; mov %0, rax; " : "=r"(ret) : "r"(sz):  "rax", "rdi") ;

     return (void*)ret;
}

void* mmap_file(const char *name)
{
      uintptr_t ret = 0;
      uint64_t n = (uint64_t)name;
     __asm__ volatile ( "mov rdi, %1; mov rax, 16; int 0x69; mov %0, rax; " : "=r"(ret) : "r"(n):  "rax", "rdi") ;

    
    return (void*)ret;
}


void* expand_heap(size_t sz){
    //uhhhhhhhh
    //lets just assume we got the size we wanted lol
    size_t size = round_up_to_page(sz);
   
    if( !brk(size)  ){
        print("libd: heap init failure");
        exit(1);
    }

    allstate.size += size;
}

void _init_heap(size_t sz){
    allstate.addr = 0;
    allstate.size = 0;
    allstate.base = 0;
    size_t size = round_up_to_page(sz);
    allstate.base = allstate.addr = brk(size);
    allstate.size += size;
    if(allstate.addr == 0){
        print("libd: heap init failure");
        exit(1);
    }

}

//just a lazy bad malloc to bootstrap for now 
void* malloc(size_t size)
{
    if(!allstate.addr || !allstate.size) //our per proc alloc state 
        _init_heap(size); //otherwise we already initialized heap to default size

    if(allstate.addr + size > (allstate.base + allstate.size)){
        expand_heap(round_up_to_page(size));
    }

    void* ret = (void*)allstate.addr;

    allstate.addr += size;

   
    return ret;
}

void free(void* p){
    //you wish
}

void *calloc(size_t num, size_t size)
{
    return malloc(num * size);
}

void *realloc(void *ptr, size_t new_size)
{
    return ptr; //and FUCKIN PRAY!
}



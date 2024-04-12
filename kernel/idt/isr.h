#pragma once
#include "../stdlib/stdint.h"
#define IRQ0 32
#define IRQ1 33
#define IRQ15 47

typedef struct {
    uint64_t ds;
    // Pushed by pusha.
    uint64_t edi, esi, ebp, ebx, edx, ecx, eax, int_no;
    uint64_t err_code, rsp; 
    //broskii theres r8-r15 too eek

    // Pushed by the processor automatically.
    uint64_t cs, eflags, eip, useresp, ss;
}  __attribute__((packed)) registers32_t;

typedef struct {
    uint64_t ds;
    // Pushed by pusha macro
    uint64_t r15, r14, r13,r12,r11,r10,r9,r8;
    uint64_t rbp, rsi, rdi, rbx, rdx, rcx, rax;
   
    //pushed by us
    uint64_t int_no, err_code;
   
    
    // Pushed by the processor automatically.
    uint64_t  rip, cs, rflags, rsp, ss;
}  __attribute__((packed)) registers_t;
//64 


/*

pushed on itrp:

cpu:

ss
prev rsp
rflags
cs
rip

err_code //us or cpu
itrp num //us

rax, rcx, rdx, rbx, rdi, rsi, rbp
r8 r9 r10 r11 r12 r13 r14 r15

----

call c handler (rdi, regs_struct)

----

pop 15,14,13,12,11,10,9,8 

*/


typedef void (*isr_t)(registers_t*);

void register_interupt_handler(uint8_t n, isr_t handler_fn); 
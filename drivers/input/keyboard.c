#include "keyboard.h"

#include "../../kernel/stdlib/stdint.h"
#include "../../kernel/stdlib/memory.h"
#include "../../kernel/idt/isr.h"
#include "../port/port.h"
#include "../video/gfx.h"
#include "../video/console.h"

#include "scancode.h"
#include "ascii.h"
#include "../../kernel/stdio/stdout.h"
//https://blog.igorw.org/2015/03/04/scancode-to-ascii/
#define SCANCODE2ASCII_TABLE /*
       0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
*/  0x00, 0x1B, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x30, 0x2D, 0x3D, 0x08, 0x09, /*0
*/  0x71, 0x77, 0x65, 0x72, 0x74, 0x79, 0x75, 0x69, 0x6F, 0x70, 0x5B, 0x5D, 0x0D, 0x00, 0x61, 0x73, /*1
*/  0x64, 0x66, 0x67, 0x68, 0x6A, 0x6B, 0x6C, 0x3B, 0x27, 0x60, 0x00, 0x5C, 0x7A, 0x78, 0x63, 0x76, /*2
*/  0x62, 0x6E, 0x6D, 0x2C, 0x2E, 0x2f, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*3
*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*4
*/  0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*5
*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*6
*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*7
*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*8
*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*9
*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*A
*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*B
*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*C
*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*D
*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /*E
*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  /*F
*/
static const unsigned char sc2ascii[] = {SCANCODE2ASCII_TABLE};

uint8_t shouldScan = 1;

//0x2a down 0xaa up
uint8_t lshift = 0;
#define KEY_QUEUE_SIZE 64
uint8_t key_queue[64];
uint8_t key_idx = 0;

static void keydown_isr(registers_t* regs){
    if(!shouldScan) return;
    uint8_t scan = port_byte_in(0x60); 
    if(scan == 0 || scan == 0xff || scan == 0x9d) return;


    if(scan == SCAN_LSHIFT_DOWN) lshift = 1;
    else if(scan == SCAN_LSHIFT_UP) lshift = 0;
   
    
    //gfx_print(htoa(sc2ascii[scan]));


    char c = sc2ascii[scan];
    //well time to read about ps2
    if(c != 0x0){
        if(lshift)
           c = apply_shift_modifier(c);
        
        if(key_idx >= KEY_QUEUE_SIZE)
            key_idx = 0;
        key_queue[key_idx] = c;
        key_idx++;
        stdout_putchar(c);
    }
        


    #ifdef LEGACY_KBD
    switch (scan){
        case 0xe:
            gfx_delc(); return;
        case 0x1c: //Enter
            c = '\n';
            break;
        default:
            c = sc2ascii[scan];
    }
    if(c == 0) return; //error! 
   
    static char b[2] = {0,0};
    b[0] = c;
   // console_print(b);
    //gfx_putc(c);
    
    #endif

 

}


void keyboard_init()
{
    register_interupt_handler(IRQ1, (isr_t)&keydown_isr);
    key_idx = 0;
    memset(key_queue, 0, KEY_QUEUE_SIZE);
}

uint8_t* get_keys(uint8_t* amt)
{
    
    uint8_t len = 0;
   // print(key_queue);
    while(key_queue[len] != 0){
        
        len++;
    }
    *amt = len;
    return key_queue;
}

bool keys_available()
{
    return key_idx > 0;
}

void keys_lock_queue()
{
    __asm__ volatile ("cli;");
    
}

void keys_reset_queue()
{
    key_idx = 0;
    __asm__ volatile ("sti;");
}

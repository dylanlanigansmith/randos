#pragma once
#include "../stdlib.h"
#define STDOUT_BUFFER 4096
#define FORCE_STDOUT

enum stdout_flags
{
    stdout_none = 0,
    stdout_backspace = (1 << 0),
    stdout_wipeoverflow = (2 << 0),
};



typedef struct 
{
    uint64_t flags;

    char* buffer;
    size_t size; 

    size_t index;

    uint8_t lock;
} stdout_t;

extern stdout_t stdout; 

bool stdout_ready();

void stdout_init();
uint8_t stdout_update();

static inline char* get_stdout(){
    stdout.buffer[stdout.index] = '\0';
   return stdout.buffer;
}
void stdout_putchar(uint8_t c);
void stdout_bytein(uint8_t byte);

static inline char stdout_top(){ return (stdout.index > 0) ?  stdout.buffer[stdout.index - 1] : stdout.buffer[0]; }
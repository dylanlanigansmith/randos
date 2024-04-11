#include "memory.h"

void* memcpy(void* dest, void* src, size_t num_bytes){
    for(int i = 0; i < num_bytes; ++i)
        *((uint8_t*)dest + i) = *((uint8_t*)src + i);
    return dest;
}


void* memset(void* dest, uint8_t val, size_t num_bytes){
    for(int i = 0; i < num_bytes; ++i){
        *((char*)(dest + i)) = val;
    }
    return dest;
}

void *memset_u32(uint32_t* dest, uint32_t val, size_t num_ints) //so we pass size as number of 4 byte chunks to write
{
    //todo alignment
    if(!dest || !num_ints) return nullptr;
    
    for(int i = 0; i < num_ints; ++i){
        dest[i] = val;
    }

    return (void*)dest;
}

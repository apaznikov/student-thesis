#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#ifndef ITEMS_COUNT
#define ITEMS_COUNT 1000000000
#endif

// Xorshift32 implementation
// A simple pseudorandom numbers generator
uint32_t xorshift32()
{
    static uint32_t x = 1;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;

    return x;
}


int main(int argc, char *argv[])
{
    uint32_t sum = 0;
#ifndef DIV
    uint32_t div = strtoul(argv[1], NULL, 10);
#endif
    
    // Generate random items with Xorshift32
    for (size_t i = 0; i < ITEMS_COUNT; ++i)
    #ifdef DIV
        sum += xorshift32() / DIV;
    #else
        sum += xorshift32() / div;
    #endif

    return sum;
}

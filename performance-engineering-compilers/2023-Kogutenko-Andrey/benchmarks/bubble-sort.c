#include <stdio.h>
#include <inttypes.h>

#ifndef ITEMS_COUNT
#define ITEMS_COUNT 50000
#endif

uint32_t array[ITEMS_COUNT];

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


// Print all array items
void print_array()
{
    for (size_t i = 0; i < ITEMS_COUNT; ++i)
        printf("%" PRIu32 " ", array[i]);

    putchar('\n');
}


int main()
{
    // Generate random items with Xorshift32
    for (size_t i = 0; i < ITEMS_COUNT; ++i)
        array[i] = xorshift32();

    // Print items before sorting
    print_array();

    // In-place sorting
    for (size_t i = 1; i < ITEMS_COUNT; ++i)
        for (size_t j = 1; j <= ITEMS_COUNT - i; ++j)
            if (array[j - 1] > array[j])
            {
                int temp = array[j - 1];
                array[j - 1] = array[j];
                array[j] = temp;
            }

    // Print items after sorting
    print_array();

    return 0;
}

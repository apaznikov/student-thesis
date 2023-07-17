#include <stdio.h>
#include <stdint.h>

int main()
{
    for (uint64_t i = 1; i <= 10000000; ++i)
    {
        uint64_t x = i;
        uint64_t iterations = 0;
        
        while (x != 1 && iterations++ <= 1000)
            if (x % 2 == 0)
                x /= 2;
            else
                x = x * 3 + 1;
        
        if (iterations == 1000)
            return 1;
    }
    
    return 0;
}

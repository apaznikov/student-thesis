#include <iostream>

int fib(int n)
{
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}

int main()
{
    int n = 33;
    std::cout << fib(n);
    return 0;
}

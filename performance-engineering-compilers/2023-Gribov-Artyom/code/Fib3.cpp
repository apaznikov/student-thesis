#include <iostream>

int fib(int n)
{
    if (n <= 1)
        return n;
    return fib(n - 1) + fib(n - 2);
}

int fib1(int n)
{
    if (n <= 1)
        return n;
    return fib(n - 1) + fib1(n - 2);
}

int fib2(int n)
{
    if (n <= 1)
        return n;
    return fib1(n - 1) + fib2(n - 2);
}
 
int main()
{
    int n = 33;
    std::cout << fib2(n);
    return 0;
}

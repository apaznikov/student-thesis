iteractions = int(input())
inputAr = []
for i in range(0, iteractions):
    inputAr.append(input())

print("""
        #include <iostream>
#include <string>
#include "heap.h"
#include <benchmark/benchmark.h>

static void BMMain(benchmark::State& state) {
    int iterations;
    Heap heap;

    iterations = """)
print(iteractions, end='')
print(";")
print("std::string inputAr[", end='')
print(iteractions, end='')
print("] = {")
for a in inputAr[:-1]:
    print('"', end='')
    print(a, end='')
    print('"', end='')
    print(",")
print('"', end='')
print(inputAr[-1], end='')
print('"', end='')
print("""
    };
    for (auto _ : state) {
        for (int i = 0; i < iterations; i++) {
            std::string input;
            input = inputAr[i];
            if (input == "GET") {
                std::cout << heap.get() << '\\n';
            } else {
                int new_value = std::stoi(input);
                heap.insert(new_value);
            }
        }
    }
}

BENCHMARK(BMMain);
BENCHMARK_MAIN();
    """)

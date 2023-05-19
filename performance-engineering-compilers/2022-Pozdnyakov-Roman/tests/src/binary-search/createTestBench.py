length = int(input())
array = list(map(int,input().strip().split()))[:length]
amount = int(input())
find = []
for i in range(0, amount):
    find.append(int(input()))

print("""
#include <iostream>
#include <benchmark/benchmark.h>

int binary_search(int* array, int left, int right, int find) {
    int middle = (left + right) / 2;

    if (array[middle] > find && middle > left)
        return binary_search(array, left, middle - 1, find);
    else if (array[middle] < find && middle < right)
        return binary_search(array, middle + 1, right, find);
    else if (array[middle] == find)
        return middle;
    else
        return -1;
}

static void BMMain(benchmark::State& state) {
    int length =
        """)
print(length)
print(";int amount = ", end='')
print(amount)
print(";int array[", end='')
print(length, end='')
print("] = {");
for a in array:
    print(a)
    if a != array[-1]:
        print(",");
print("};int find[", end='')
print(amount, end='')
print("] = {")
for a in find:
    print(a)
    if a != find[-1]:
        print(",");
print("""
};
    for (auto _ : state) {
        for (int i = 0; i < amount; i++) {
            binary_search(array, 0, length, find[i]);
        }
    }
}

BENCHMARK(BMMain);
BENCHMARK_MAIN();
    """)


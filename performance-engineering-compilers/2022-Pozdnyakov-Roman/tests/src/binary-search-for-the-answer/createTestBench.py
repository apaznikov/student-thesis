n = int(input())
k = int(input())
array = []
for i in range(0, n):
    array.append(int(input()))

print("""
        #include <iostream>
#include <benchmark/benchmark.h>

int test_line_length(int* array, int length, int line_length, int required_dots_amount) {
    int current_dot = 0;
    int test_dots_amount = 0;
    
    while (current_dot < length) {
        test_dots_amount++;
        int line_end = array[current_dot] + line_length;
        current_dot++;
        while (current_dot < length && array[current_dot] <= line_end)
            current_dot++;
    }
    return test_dots_amount <= required_dots_amount;
}

int binary_search_for_the_answer(int* array, int length, int dots_amount) {
    int left = array[0];
    int right = array[length - 1];
    int test_length = 0;
    int last_best = 0;
    
    while (right > left + 1) {
        int middle = (right + left) / 2;
        test_length = middle - array[0];

        if (test_line_length(array, length, test_length, dots_amount)) {
            last_best = test_length;
            right = middle;
        } else
            left = middle;
    }
    return last_best;
}

static void BMMain(benchmark::State& state) {
    int n, k;
    int answer;
    
    n = """)
print(n, end='')
print(";")
print("k = ", end='')
print(k)
print(";int array[", end='')
print(n, end='')
print("] = {", end='')
for i in array:
    print(i)
    if i != array[-1]:
        print(",");
print("""
    };
    for (auto _ : state) {
        answer = binary_search_for_the_answer(array, n, k);
    }
    std::cout << answer << '\\n';
}

BENCHMARK(BMMain);
BENCHMARK_MAIN();
    """)

#include <algorithm>
#include <cinttypes>
#include <iostream>
#include <fstream>
#include <vector>

#include <time.h>

void bubble_sort(std::vector<int> &arr);

int main(void) {
    std::vector<int> arr;

    {
        std::ifstream file{ "data.txt" };
        if (!file) return EXIT_FAILURE;

        size_t size;
        file >> size;

        arr.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            int tmp; file >> tmp;
            arr.push_back(tmp);
        }
    }

    #if !defined(NO_PRINT)
    auto printer = [](auto elem) { std::cout << elem << " "; };
    std::cout << "Before:\n";
    std::for_each(arr.cbegin(), arr.cend(), printer);
    std::cout << "\n";
    #endif

    struct timespec start, end;
    int clock_start = clock_gettime(CLOCK_MONOTONIC, &start);
    bubble_sort(arr);
    int clock_end = clock_gettime(CLOCK_MONOTONIC, &end);

    if (clock_start == -1 || clock_end == -1) {
        puts("ERROR! Can't get time!");
        return EXIT_FAILURE;
    }

    #if !defined(NO_PRINT)
    std::cout << "After:\n";
    std::for_each(arr.cbegin(), arr.cend(), printer);
    std::cout << "\n";
    #endif

    size_t start_time = start.tv_sec*1000 + start.tv_nsec/1000000;
    size_t end_time = end.tv_sec*1000 + end.tv_nsec/1000000;
    fprintf(stderr, "Time of execution = %zu ms\n", end_time - start_time);

    return EXIT_SUCCESS;
}

void bubble_sort(std::vector<int> &arr) {
    for (size_t i = 1, end = arr.size(); i < end; ++i) {
        for (size_t j = 1; j <= end - i; ++j) {
            if (arr[j - 1] > arr[j])
                std::swap(arr[j - 1], arr[j]);
        }
    }
}


#include <iostream>
#include "../tree/tree_node.h"
#include <benchmark/benchmark.h>

static void BMMain(benchmark::State& state) {
    int actionsAmount;
    Tree_node tree;

    actionsAmount = 
6;
int actions[6] = {417
,
274
,
417
,
53
,
274
,
89

};
    for (auto _ : state) {
        for (int i = 0; i < actionsAmount; i++) {
            int input = actions[i];
            Tree_node* node = tree.insert(input);
            if (node) {
                std::cout << "-\n";
            } else {
                std::cout << "+\n";
            }
        }
    }
}

BENCHMARK(BMMain);
BENCHMARK_MAIN();
    

actionsAmount = int(input())
actions = []
for i in range(0, actionsAmount):
    actions.append(int(input()))

print("""
#include <iostream>
#include "../tree/tree_node.h"
#include <benchmark/benchmark.h>

static void BMMain(benchmark::State& state) {
    int actionsAmount;
    Tree_node tree;

    actionsAmount = """)
print(actionsAmount, end='')
print(";")
print("int actions[", end = '')
print(actionsAmount, end='')
print("] = {", end='')
for a in actions:
    print(a)
    if a != actions[-1]:
        print(",")
print("""
};
    for (auto _ : state) {
        for (int i = 0; i < actionsAmount; i++) {
            int input = actions[i];
            Tree_node* node = tree.insert(input);
            if (node) {
                std::cout << "-\\n";
            } else {
                std::cout << "+\\n";
            }
        }
    }
}

BENCHMARK(BMMain);
BENCHMARK_MAIN();
    """)

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
            int contains = 0;
            int has_next = 0;
            Tree_node* node = tree.insert(input);
            contains = node ? 1 : 0;
            if (!node)
                node = tree.get(input);
            Tree_node* next = node->next();

            if (contains)
                std::cout << "- ";
            else
                std::cout << "+ ";
            if (next)
                std::cout << next->get_id() << '\\n';
            else
                std::cout << "-\\n";
        }
    }
}

BENCHMARK(BMMain);
BENCHMARK_MAIN();
    """)

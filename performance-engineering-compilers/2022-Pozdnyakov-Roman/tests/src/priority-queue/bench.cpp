
        #include <iostream>
#include <string>
#include "heap.h"
#include <benchmark/benchmark.h>

static void BMMain(benchmark::State& state) {
    int iterations;
    Heap heap;

    iterations = 
131;
std::string inputAr[131] = {
"701",
"84",
"GET",
"GET",
"342",
"787",
"GET",
"238",
"360",
"GET",
"16",
"GET",
"37",
"578",
"185",
"839",
"782",
"480",
"GET",
"539",
"914",
"GET",
"345",
"GET",
"GET",
"645",
"467",
"268",
"GET",
"623",
"GET",
"298",
"444",
"211",
"772",
"GET",
"653",
"643",
"174",
"GET",
"586",
"GET",
"820",
"GET",
"32",
"GET",
"684",
"322",
"840",
"495",
"415",
"12",
"90",
"GET",
"13",
"828",
"GET",
"905",
"294",
"344",
"682",
"797",
"843",
"853",
"312",
"193",
"544",
"551",
"600",
"GET",
"557",
"522",
"400",
"933",
"GET",
"407",
"429",
"534",
"GET",
"234",
"494",
"506",
"975",
"497",
"GET",
"911",
"396",
"928",
"673",
"661",
"610",
"809",
"867",
"GET",
"741",
"740",
"587",
"GET",
"75",
"GET",
"GET",
"488",
"271",
"61",
"GET",
"567",
"440",
"851",
"857",
"683",
"660",
"372",
"743",
"525",
"513",
"GET",
"GET",
"984",
"129",
"654",
"935",
"351",
"145",
"21",
"498",
"737",
"383",
"GET",
"815",
"533",
"GET"
    };
    for (auto _ : state) {
        for (int i = 0; i < iterations; i++) {
            std::string input;
            input = inputAr[i];
            if (input == "GET") {
                std::cout << heap.get() << '\n';
            } else {
                int new_value = std::stoi(input);
                heap.insert(new_value);
            }
        }
    }
}

BENCHMARK(BMMain);
BENCHMARK_MAIN();
    

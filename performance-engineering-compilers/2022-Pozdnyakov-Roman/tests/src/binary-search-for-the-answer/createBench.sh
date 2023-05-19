#!/usr/bin/env bash
IFS=$'\n'

test=$1

python3 createTestBench.py < $test > bench.cpp
clang++ bench.cpp -o bench -Wl,--emit-relocs -isystem benchmark/include \
  -Lbenchmark/build/src -lbenchmark -lpthread

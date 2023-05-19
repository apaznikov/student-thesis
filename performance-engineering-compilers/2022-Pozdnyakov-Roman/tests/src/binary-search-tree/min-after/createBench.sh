#!/usr/bin/env bash
IFS=$'\n'

test=$1

python3 createTestBench.py < $test > bench.cpp

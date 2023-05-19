#!/bin/bash
set -e
gcc test_list_NoTail_back.c -g -O0 -no-pie -falign-functions=512 -o test_list_NoTail_back
halo baseline --trials 10 --pmu-events L1-dcache-load-misses \
              --directory results/test_list_NoTail_back -- ./test_list_NoTail_back
halo run --trials 10 --pmu-events L1-dcache-load-misses \
         --affinity-distance 128 --directory results/test_list_NoTail_back \
         -- ./test_list_NoTail_back \
         -- ./test_list_NoTail_back
halo plot --save-as results/test_list_NoTail_back/L1d.pdf --metric 'L1-dcache-load-misses@negimprovement' \
          --y-label 'L1D Cache Miss Reduction' --group 2 \
          --group-label malloc --group-label HALO --label workload_nam \
          results/test_list_NoTail_back/baseline/results-malloc-* \
          results/test_list_NoTail_back/affinity-128*/results-malloc-*
halo plot --save-as results/test_list_NoTail_back/Time.pdf --metric 'time elapsed@speedup' \
          --y-label 'Time Reduction' --group 2                  \
          --group-label malloc --group-label HALO --label workload_nam \
          results/test_list_NoTail_back/baseline/results-malloc-* \
          results/test_list_NoTail_back/affinity-128*/results-malloc-*
# plot metric:
# 'time elapsed'
# 'L1-dcache-load-misses'
# optionally:
# '@relative'
# '@speedup'
# '@negimprovement'
# '@improvement'

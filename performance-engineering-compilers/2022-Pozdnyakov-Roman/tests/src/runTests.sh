#!/usr/bin/env bash
IFS=$'\n'

progs="
binary-search
binary-search-for-the-answer
priority-queue
binary-search-tree/contains
binary-search-tree/min-after
"

> times.txt
rootLocation=$(pwd)
times=$(pwd)/times.txt

for prog in $progs
do
    cd $rootLocation
    cd $prog
    echo $prog >> $times
    echo default >> $times
    ./compile.sh
    (time ./test.sh) &>> $times
    ./optimize.sh
    echo optimizes >> $times
    (time ./test.sh) &>> $times
done

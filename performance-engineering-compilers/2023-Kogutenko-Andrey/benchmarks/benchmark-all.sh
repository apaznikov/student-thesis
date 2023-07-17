#!/bin/bash

DYNAMORIO_PATH="/home/somebody/DynamoRIO-Linux-9.0.1"
ICC_PATH="/home/somebody/intel/oneapi/compiler/latest"

TIMEFORMAT="%E"
DRRUN="$DYNAMORIO_PATH/bin64/drrun"
ICC="$ICC_PATH/linux/bin/icx"

function bench
{
    echo -n "Source     "; time "./$1" "$2" > /dev/null
    echo -n "QEMU       "; time qemu-x86_64 "./$1" "$2" > /dev/null
    echo -n "DynamoRIO  "; time "$DRRUN" "./$1" "$2" > /dev/null
    echo -n "Valgrind   "; time valgrind "./$1" "$2" 2> /dev/null > /dev/null
}



echo "loop-alignment"

# Building non-optimized version
nasm -f elf64 loop-alignment.asm -D ALIGNMENT=63 -D INNER_NOPS=4
ld -o loop-alignment loop-alignment.o
bench "loop-alignment"

# Building optimized version
nasm -f elf64 loop-alignment.asm -D ALIGNMENT=64 -D INNER_NOPS=4
ld -o loop-alignment-optimized loop-alignment.o
rm loop-alignment.o

echo -n "Optimized  "; time ./loop-alignment-optimized > /dev/null

echo



echo "loop-unrolling"

# Building non-optimized version
nasm -f elf64 loop-unrolling.asm
ld -o loop-unrolling loop-unrolling.o
bench "loop-unrolling"

# Building optimized version
nasm -f elf64 loop-unrolling.asm -D INC
ld -o loop-unrolling-optimized loop-unrolling.o
rm loop-unrolling.o

echo -n "Optimized  "; time ./loop-unrolling-optimized > /dev/null

echo



echo "branches"

# Building non-optimized version
nasm -f elf64 branches.asm
ld -o branches branches.o
bench "branches"

# Building optimized version
nasm -f elf64 branches.asm -D BRANCHLESS
ld -o branches-optimized branches.o
rm branches.o

echo -n "Optimized  "; time ./branches-optimized > /dev/null

echo



echo "collatz"

# Version with variable divisor
echo "GCC O2"
gcc -O2 -o "collatz-gcc" collatz.c
bench "collatz-gcc" 16

echo "Clang O2"
clang -O2 -o "collatz-clang" collatz.c
bench "collatz-clang" 16

echo "ICC O2"
"$ICC" -O2 -o "collatz-icc" collatz.c
bench "collatz-icc" 16

# Version with constant divisor
echo "GCC Ofast march=native"
gcc -Ofast -march=native -o "collatz-gcc" collatz.c
bench "collatz-gcc" 16

echo "Clang Ofast march=native"
clang -Ofast -march=native -o "collatz-clang" collatz.c
bench "collatz-clang" 16

echo "ICC Ofast march=native"
clang -Ofast -march=native -o "collatz-icc" collatz.c
bench "collatz-icc" 16

echo



echo "data-dependence"

# Version with variable divisor
echo "GCC"
gcc -O2 -o "data-dependence-gcc" data-dependence.c
bench "data-dependence-gcc" 16

echo "Clang"
clang -O2 -o "data-dependence-clang" data-dependence.c
bench "data-dependence-clang" 16

echo "ICC"
"$ICC" -O2 -o "data-dependence-icc" data-dependence.c
bench "data-dependence-icc" 16

# Version with constant divisor
echo "GCC const"
gcc -O2 -o "data-dependence-gcc" data-dependence.c -D DIV=16
bench "data-dependence-gcc" 16

echo "Clang const"
clang -O2 -o "data-dependence-clang" data-dependence.c -D DIV=16
bench "data-dependence-clang" 16

echo "ICC const"
clang -O2 -o "data-dependence-icc" data-dependence.c -D DIV=16
bench "data-dependence-icc" 16

echo



echo "bubble-sort"

# Without optimizations
echo "GCC O0"
gcc -O0 -o "bubble-sort-gcc" bubble-sort.c
bench "bubble-sort-gcc"

echo "Clang O0"
clang -O0 -o "bubble-sort-clang" bubble-sort.c
bench "bubble-sort-clang"

echo "ICC O0"
"$ICC" -O0 -o "bubble-sort-icc" bubble-sort.c
bench "bubble-sort-icc"

# With optimizations
echo "GCC O2"
gcc -O2 -o "bubble-sort-gcc" bubble-sort.c
bench "bubble-sort-gcc"

echo "Clang O2"
clang -O2 -o "bubble-sort-clang" bubble-sort.c
bench "bubble-sort-clang"

echo "ICC O2"
"$ICC" -O2 -o "bubble-sort-icc" bubble-sort.c
bench "bubble-sort-icc"

# With optimizations without vectorization
echo "GCC O2 fno-tree-vectorize"
gcc -O2 -fno-tree-vectorize -o "bubble-sort-gcc" bubble-sort.c
bench "bubble-sort-gcc"

echo "Clang O2 fno-tree-vectorize"
clang -O2 -fno-tree-vectorize -o "bubble-sort-clang" bubble-sort.c
bench "bubble-sort-clang"

echo "ICC O2 fno-tree-vectorize"
"$ICC" -O2 -fno-tree-vectorize -o "bubble-sort-icc" bubble-sort.c
bench "bubble-sort-icc"

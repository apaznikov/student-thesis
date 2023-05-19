#!/usr/bin/env bash

name=priority-queue
files="main.cpp heap.cpp"
cc=clang++

$cc $files -Ofast -o $name -Wl,--emit-relocs

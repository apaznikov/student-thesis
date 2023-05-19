#!/usr/bin/env bash

name=binary-search-tree
files="main.cpp ../tree/tree_node.cpp"
cc=clang++

$cc $files -Ofast -o $name -Wl,--emit-relocs

#!/usr/bin/env bash
IFS=$'\n'

name=binary-search-for-the-answer
files=main.cpp
cc=clang++

$cc $files -o $name -Wl,--emit-relocs

#!/usr/bin/env bash
IFS=$'\n'

name=binary-search
files=main.cpp
cc=clang++

$cc $files -o $name -Wl,--emit-relocs

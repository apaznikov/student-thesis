#!/usr/bin/env bash
IFS=$'\n'

name=binary-search-tree
test="./../../../${name}/1.in"

llvm-bolt $name -instrument -o instrumented
./instrumented < $test
rm instrumented
mv /tmp/prof.fdata prof.fdata
llvm-bolt $name -o $name.bolt -data=prof.fdata -reorder-blocks=cache+ -reorder-functions=hfsort -split-functions=2 -split-all-cold -split-eh -dyno-stats
rm $name
mv $name.bolt $name

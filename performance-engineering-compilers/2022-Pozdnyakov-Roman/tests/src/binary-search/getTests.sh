#!/usr/bin/env bash
IFS=$'\n'

name=binary-search

tests=$(find ./../../$name/ -name *.in)
tests=$(echo $tests | tr " " "\n" | sort)
echo $tests

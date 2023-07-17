#!/bin/bash

export LC_ALL=en_US.UTF-8
TIMEFORMAT="%E"

ALIGNMENT_RANGE="59 60 61 62 63 64"
INNER_NOPS_RANGE="0 1 2 3 4 5"

echo "$INNER_NOPS_RANGE"

for ALIGNMENT in $ALIGNMENT_RANGE
do
    echo -n "$ALIGNMENT "

    for INNER_NOPS in $INNER_NOPS_RANGE
    do
        rm -f loop-alignment
        make loop-alignment DEFINES="-D ALIGNMENT=$ALIGNMENT -D INNER_NOPS=$INNER_NOPS" > /dev/null
        { time ./loop-alignment; } 2>&1 | tr '\n' ' '
    done
    echo
done

#!/usr/bin/env bash

#
# This script runs regression testing on small hand-written test programs.
#


SRC=(
    "basic"
    "assign"
    "dereference"
    "func"
    "cast"
    "array"
    "addressof"
    # "checkable"
    "stack_global"
)

#
# Entrance of this script
#
if [[ $# == 1 ]]; then
    # Compile and run one test file.
    make $1
    echo "Running test on $1"
    ./$1
else
    # compile and run all the regression testing files
    make rt
    for src in ${SRC[@]}; do
        echo "Running test on $src"
        ./$src
    done
fi


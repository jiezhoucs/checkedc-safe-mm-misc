#!/usr/bin/env bash

#
# This script runs regression testing on small hand-written test programs.
#


SRC=(
    "basic"
    "assign"
    "dereference"
    "func"
)

#
# Entrance of this script
#

# compile all the regression testing files
make rt

# run
for src in ${SRC[@]}; do
    echo "Running testing on $src"
    ./$src
done

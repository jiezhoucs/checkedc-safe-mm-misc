#!/usr/bin/env bash

#
# This script runs regression testing on small hand-written test programs.
#


SRC=(
    "assign"
    "dereference"
    # "funccall"
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

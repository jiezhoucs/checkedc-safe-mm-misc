#!/bin/bash

#
# This scritps run the benchmarks of the original Olden test suite.
#

# load common directory paths and variables
. common.sh

#
# Olden-specific paths
BUILD_DIR="$TESTS_DIR/ts-build-baseline"
DATA_DIR="$DATA_DIR/olden/baseline"
BIN_DIR="$BUILD_DIR/MultiSource/Benchmarks/Olden"

PROGRAMS=(
    "bh"
    "bisort"
    "em3d"
    "health"
    "mst"
    "perimeter"
    "power"
    "treeadd"
    "tsp"
    # "voronoi"
)

cd $BUILD_DIR

#
# Run one single benchmark program.
#
run_one() {
    # Compile the benchmark if its binary does not exist.
    if [[ ! -f "$BIN_DIR/$1/$1" ]]; then
        echo "Compiling $1..."
        make -j$PARA_LEVEL $1
    fi

    $LIT -v --filter $1 -o $DATA_DIR/$1.json .
}

#
# Run all benchmarks in Olden
#
run_all() {
    for prog in ${PROGRAMS[@]}; do
        run_one $prog
    done
}

#
# Clean all compiled binaries.
#
clean() {
    echo "Cleaning all original Olden benchmark binaries..."
    for prog in ${PROGRAMS[@]}; do
        echo "Cleaning $prog"
        cd "$BIN_DIR/$prog"
        find . -name "*.o" -delete
        rm -f "$prog" "$prog.stripped"
    done
}

#
# Print out usgae and exit.
#
usage() {
    echo "Usage $0 <benchmark>"
    echo
    echo "If no argument is given, $0 runs all the benchmark programs in Olden."
    echo "To run a single benchmark program, specify its name as the first"\
        "argument of $0."
    exit
}

#
# Entrance of this script
#

# mkdir the data directory if it does not exist.
[[ -d $DATA_DIR ]] || mkdir -p $DATA_DIR

if [[ $# == 1 ]]; then
    if [[ $1 == "-h" || $1 == "--help" ]]; then
        usage
    elif [[ $1 == "clean" ]]; then
        clean
    else
        run_one $1
    fi
else
    run_all
fi

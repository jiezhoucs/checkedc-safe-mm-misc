#!/bin/bash

#
# This scritps run the benchmarks of the modified Olden test suite.
#

# load common directory paths and variables
. common.sh

#
# Olden-specific paths
BUILD_DIR="$TESTS_DIR/ts-build"
DATA_DIR="$DATA_DIR/olden/checkedc"
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
    "voronoi"
)

cd $BUILD_DIR

#
# Run one single benchmark program.
#
run_one() {
    # Compile the benchmark if its binary does not exist.
    if [[ ! -f "$BIN_DIR/$1/$1" ]]; then
        echo "Compiling $1..."
        # Set the compile parallel level to be #ofLogicalCore - 2.
        local parallell
        if [[ $OS == "Linux" ]]; then
            parallell=`lscpu | grep "^CPU(s)" | cut -d ':' -f2 | echo "$(cat -)-2" | bc`
        elif [[ $OS == "Darwin" ]]; then
            parallell=`sysctl -n hw.ncpu | echo "$(cat -)-2" | bc`
        fi
        make -j$parallell $1
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
    for prog in ${PROGRAMS[@]}; do
        cd "$BIN_DIR"
        rm -f "$prog/$prog"
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

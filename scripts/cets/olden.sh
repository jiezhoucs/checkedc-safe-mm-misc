#!/bin/bash

#
# This scritps run the benchmarks of the Olden test suite.
#

#
# cd to the CETS directory that contains the script for running CETS.
#
ROOT_DIR=`dirname $0 | sed 's/$/\/..\/..\/../' | xargs realpath`
cd "$ROOT_DIR/misc/scripts/cets"

# load common directory paths and variables
. common.sh

LLVM_TS_BIN_DIR="$LLVM_TS_BUILD/MultiSource/Benchmarks/Olden"

PROGRAMS=(
    # "bh"
    "bisort"
    # "em3d"
    "health"
    # "mst"
    "perimeter"
    "power"
    "treeadd"
    "tsp"
    # "voronoi"
)

cd $LLVM_TS_BUILD

#
# Run one single benchmark program.
#
run_one() {
    # Compile the benchmark if its binary does not exist.
    if [[ ! -f "$LLVM_TS_BIN_DIR/$1/$1" ]]; then
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

    $CETS_LIT -v --filter $1 -o $DATA_DIR/$1.json .
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
    echo "Cleaning all Olden benchmark binaries..."
    for prog in ${PROGRAMS[@]}; do
        echo "Cleaning $prog"
        cd "$LLVM_TS_BIN_DIR/$prog"
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

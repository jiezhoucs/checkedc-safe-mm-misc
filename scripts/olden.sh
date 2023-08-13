#!/bin/bash

#
# This script runs the benchmarks of the Olden test suite.
#

set -e

# load common directory paths and variables
. $(dirname $0)/common.sh

BENCHMARKS=(
    "bh"
    "bisort"
    "em3d"
    "health"
    "mst"
    "perimeter"
    "power"
    "treeadd"
    "tsp"
)

#
# Check if a user-provided benchmark name is valid
#
benchmark_exists() {
    for benchmark in ${BENCHMARKS[@]}; do
        if [[ $benchmark == $1 ]]; then
            return 0
        fi
    done

    return 1
}

#
# Run one single benchmark program.
#
run_benchmark() {
    if ! benchmark_exists $1; then
        echo "Invalid Olden benchmark name!"
        exit 1
    fi

    cd "$BUILD_DIR"
    # Compile the benchmark if its binary does not exist.
    if [[ ! -f "$BIN_DIR/$1/$1" ]]; then
        echo "Compiling $TARGET $1 ..."
        make -j$PARA_LEVEL $1 || { echo "Failed to compile $1!"; exit 1; }
    fi

    echo "Running $TARGET $1 ..."
    $LIT -v --filter $1 -o $DATA_DIR/$1.json .
}

#
# Run all benchmarks of Olden.
#
run_all() {
    for benchmark in ${BENCHMARKS[@]}; do
        run_benchmark $benchmark
    done
}

#
# Clean all compiled binaries.
#
clean() {
    echo "Cleaning all Olden benchmark binaries ..."
    for benchmark in ${BENCHMARKS[@]}; do
        echo "Cleaning $benchmark"
        cd "$BIN_DIR/$benchmark"
        find . -name "*.o" -delete
        rm -f "$benchmark" "$benchmark.stripped"
    done
    exit
}

#
# Print out usgae and exit.
#
usage() {
    echo "Usage: "
    echo "  ./olden.sh [target] [benchmark] | clean"
    echo
    echo "  target: baseline, checked, or cets"
    echo "  benchmark: Olden benchmark. If no benchmark name is given, all benchmarks will run."
    echo "  \"clean\" removes exsiting binaries"
    exit
}

main() {
    case $1 in
        -h|--help)
            usage
            ;;
        # Set env according to the build target.
        baseline)
            BUILD_DIR="$TESTS_DIR/ts-build-baseline"
            DATA_DIR="$DATA_DIR/olden/baseline"
            LIT="$LLVM_VANILLA_BIN/llvm-lit"
            ;;
        checked)
            BUILD_DIR="$TESTS_DIR/ts-build"
            DATA_DIR="$DATA_DIR/olden/checked"
            LIT="$LLVM_BIN_DIR/llvm-lit"
            ;;
        cets)
            BUILD_DIR="$TESTS_DIR/ts-build-cets"
            DATA_DIR="$DATA_DIR/olden/cets"
            LIT="$CETS_BIN/llvm-lit"
            BENCHMARKS=(
                # "bh", "em3d" and "mst" cannot compile or run with CETS
                "bisort"
                "health"
                "perimeter"
                "power"
                "treeadd"
                "tsp"
            )
            ;;
        *)
            echo "Unknown argument(s)!"
            usage
            ;;
    esac

    TARGET="$1"

    # Set the directory containing the executables.
    BIN_DIR="$BUILD_DIR/MultiSource/Benchmarks/Olden"

    # mkdir the data directory if it does not exist.
    mkdir -p "$DATA_DIR"

    if [[ $# == 2 ]]; then
        if [[ $2 == "clean" ]]; then
            clean
        fi

        run_benchmark $2
    else
        run_all
    fi
}

#
# Entrance of this script
#
main $1 $2
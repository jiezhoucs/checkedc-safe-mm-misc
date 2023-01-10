#!/usr/bin/env bash

#
# This script runs the baseline and the checked curl for performance evaluation.
#
# $1: "baseline or "checked"
#

. common.sh

# Excluded tests.
EXCLUDED="!1014 !1119 !1135 !1167"
ITER=20
DATA_DIR="$DATA_DIR/curl"

#
# Init the experiment settings and run runtests.pl.
#
run() {
    if [[ $1 == "baseline" ]]; then
        echo "Run the baseline curl"
        CURL_DIR="$BENCHMARKS_DIR/baseline/curl"
        DATA_DIR="$DATA_DIR/baseline"
    else
        echo "Run the checked curl"
        CURL_DIR="$BENCHMARKS_DIR/checked/curl"
        DATA_DIR="$DATA_DIR/checked"
    fi

    # Prepare the result directory.
    if [[ ! -d $DATA_DIR ]]; then
        mkdir -p $DATA_DIR
    fi
    rm -rf $DATA_DIR/*

    cd $CURL_DIR
    # Check if executables exist, and build them if not.
    if [[ ! -f src/curl ]]; then
        make -j
    fi
    # Check and build tests.
    cd tests
    make -j

    # Run.
    for i in $(seq 1 $ITER); do
        echo "Running runtests.pl to evaluate curl."
        echo "Iteration $i..."
        ./runtests.pl $EXCLUDED > "$DATA_DIR/result.$i"
        echo "Finished iteration %1."
    done
}


#
# Compute the average execution time is:
#
perf() {
    cd $DATA_DIR
    grep considered * | cut -d' ' -f7 | paste  -sd+ - | bc
}

#
# Entrance of this script.
#
run $1

perf

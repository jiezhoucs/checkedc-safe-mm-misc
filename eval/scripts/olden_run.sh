#!/usr/bin/env bash

#
# This script runs the baseline, checked, or CETS Olden benchmark for
# performance evaluation.
#

. common.sh

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

ITER=20

cd "$MISC_DIR/scripts"

#
# Run the script that runs Olden benchmarks for a setup.
#
run() {
    # Prepare script and data directory.
    if [[ $1 == "baseline" ]]; then
        olden_script="./olden-baseline.sh"
        data_dir="$DATA_DIR/olden/baseline"
    elif [[ $1 == "cets" ]]; then
        olden_script="cets/olden.sh"
        data_dir="$DATA_DIR/olden/cets"
    else
        olden_script="./olden.sh"
        data_dir="$DATA_DIR/olden/checked"
    fi

    # Clean old data
    rm -rf $data_dir/*

    for i in $(seq 1 $ITER); do
        # Run the script to run Olden benchmarks
        cd $MISC_DIR/scripts
        $olden_script

        # Rename result files.
        cd $data_dir
        for benchmark in ${BENCHMARKS[@]}; do
            if [[ -f $benchmark.json ]]; then
                mv $benchmark.json $benchmark.$i.json
            fi
        done
    done
}

#
# Entrance of this script.
#
run $1

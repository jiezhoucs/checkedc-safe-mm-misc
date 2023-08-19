#!/usr/bin/env bash

#
# This script runs the baseline, checked, or CETS Olden benchmark for
# performance evaluation.
#
# $1: "baseline", "checked", or "cets"
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

#
# Run the script that runs Olden benchmarks for a target.
#
main() {
    case $1 in
        "baseline"|"checked"|"cets")
            target=$1
            ;;
        *)
            echo "Unknow target!"
            exit 1
            ;;
    esac

    # Set script and result data directory.
    olden_script="$MISC_DIR/scripts/olden.sh"
    data_dir="$DATA_DIR/olden/$target"

    # Clean old data
    rm -rf $data_dir/*

    for i in $(seq 1 $ITER); do
        # Run Olden benchmarks
        $olden_script $target

        # Rename result files.
        for benchmark in ${BENCHMARKS[@]}; do
            if [[ -f "$data_dir/$benchmark.json" ]]; then
                mv "$data_dir/$benchmark.json" "$data_dir/$benchmark.$i.json"
            fi
        done
    done
}

#
# Entrance of this script.
#
main $1
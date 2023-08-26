#!/usr/bin/env bash

#
# This script runs the baseline, checked, or CETS Olden benchmark for
# performance evaluation.
#
# $1: "baseline", "checked", or "cets"
#

set -e

. common.sh

DATA_DIR="$EVAL_DIR/perf_data/olden"
BENCHMARKS=$OLDEN_BENCHMARKS

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
    data_dir="$DATA_DIR/$target"

    mkdir -p $data_dir
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
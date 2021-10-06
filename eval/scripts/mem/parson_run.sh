#!/usr/bin/env bash

#
# This script runs the baseline and Checked C parson for the purpose of
# memory consumption measurement.
#
# $1 - "baseline", "checked", or "cets".
#

. common.sh

DATA_DIR="$DATA_DIR/parson"
BASELINE_PARSON_DIR="$BENCHMARK_DIR/baseline/parson"
CHECKED_PARSON_DIR="$BENCHMARK_DIR/checked/parson"

DATA_FILES=(
    "countries-small"
    "profiles"
    "covers"
    "books"
    "albums"
    "restaurant"
    "countries-big"
    "zips"
    "images"
    "city_inspections"
    "companies"
    "trades"
    "citylots"
)

#
#  Run parson's eval program and wss.pl.
#
run() {
    # Prepare directories.
    if [[ $1 == "baseline" ]]; then
        parson_dir=$BASELINE_PARSON_DIR
        data_dir="$DATA_DIR/baseline"
    else
        parson_dir=$CHECKED_PARSON_DIR
        data_dir="$DATA_DIR/checked"
    fi
    mkdir -p $data_dir
    rm -rf $data_dir/*
    cd $parson_dir

    # Run the evaluation binary and collect memory consumption data.
    for data in ${DATA_FILES[@]}; do
        echo "Measuring memory consumption for $data"
        ./eval $data & $WSS -s 0 $! 0.05 >& $data_dir/$data.stat || true
    done
}

#
# Entrance of this script.
#
run $1

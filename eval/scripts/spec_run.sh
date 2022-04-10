#!/usr/bin/env bash

#
# This script runs 505.mcf_r and 519.lbm_r of SPEC CPU2017
#

. common.sh

BUILD_DIR="$ROOT_DIR/benchmark-build/spec"
DATA_DIR="$DATA_DIR/spec"

PROGRAMS=(
    "505.mcf_r"
    "519.lbm_r"
)

#
# Run
#
run() {
    if [[ $1 == "baseline" ]]; then
        DATA_DIR="$DATA_DIR/baseline"
        BUILD_DIR="$BUILD_DIR/baseline"
    else
        DATA_DIR="$DATA_DIR/checked"
        BUILD_DIR="$BUILD_DIR/checked"
    fi
    mkdir -p $DATA_DIR
    rm -rf $DATA_DIR/*

    cd $BUILD_DIR
    for prog in ${PROGRAMS[@]}; do
        echo "Running $prog..."
        $LIT -vv --filter $prog -o $DATA_DIR/$prog.json .
    done
}

#
# Entrance
#
run $1

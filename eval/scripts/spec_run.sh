#!/usr/bin/env bash

#
# This script runs 429.mcf and 470.lbm of SPEC CPU2006
#
# $1 - "baseline" or "checked"(optional).
#

. common.sh

BUILD_DIR="$ROOT_DIR/benchmark-build/spec"
DATA_DIR="$DATA_DIR/spec"

PROGRAMS=(
    "429.mcf"
    # "470.lbm"
)

ITER=20

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
    for i in $(seq 1 $ITER); do
        for prog in ${PROGRAMS[@]}; do
            echo "Running $prog..."
            $LIT -vv --filter $prog -o $DATA_DIR/$prog.$i.json .
        done
    done
}

#
# Entrance
#
run $1

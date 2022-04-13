#!/usr/bin/env bash

#
# This script runs 429.mcf and uses wss to collect memory consumption data.
#
# $1 - "baseline" or "checked"(optional).
#

. common.sh

BUILD_DIR="$ROOT_DIR/benchmark-build/spec"
DATA_DIR="$DATA_DIR/spec"
LIT="$ROOT_DIR/build/bin/llvm-lit"

PROGRAMS=(
    "429.mcf"
    # "470.lbm"
)
PROGRAM="429.mcf"

run() {
    # configure
    if [[ $1 == "baseline" ]]; then
        data_dir="$DATA_DIR/baseline"
        build_dir="$BUILD_DIR/baseline"
    else
        data_dir="$DATA_DIR/checked"
        build_dir="$BUILD_DIR/checked"
    fi
    mkdir -p $data_dir
    rm -rf $data_dir/*

    cd $build_dir
    $LIT -vv --filter $PROGRAM . >& /dev/null &

    pid=
    while [[ ! $pid ]]; do
        pid=`pgrep -n -U $UID 429.mcf`
    done
    echo "Running wss to collect memory data"
    $WSS -s -0 $pid 1 >& $data_dir/429.mcf || true
}

#
# Entrance of this script.
#
run $1


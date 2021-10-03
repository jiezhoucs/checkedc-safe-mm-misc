#!/usr/bin/env bash

#
# This script analyzes selected C applications in LLVM test-suite.
#

. common.sh

BUILD_DIR="$ANALYSIS_DIR/ts-build"
DATA_DIR="$ANALYSIS_DIR/data/ts-apps"

APPS=(
    "clamscan"
    "lua"
)

compile() {
    cd $BUILD_DIR
    echo "Compiling $1"
    make $1 -j8
}

run() {
    rm -f /tmp/analysis_result.txt
    cd $BUILD_DIR
    echo "Running $1..."
    $LIT -vv --filter $1 -o "$DATA_DIR/$1.json" .
}

if [[ $# == 1 ]]; then
    if [[ $1 == "ls" ]]; then
        for prog in ${APPS[@]}; do
            echo $prog
        done
    else
        compile $1
    fi
elif [[ $# == 2 && $2 == "run" ]]; then
    compile $1
    run $1
fi

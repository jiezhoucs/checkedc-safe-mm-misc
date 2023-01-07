#!/usr/bin/env bash

# This script prints out the experimental results.
#
# $1: benchmark name.

ROOT_DIR=`realpath .`
EVAL_SCRIPTS_DIR="$ROOT_DIR/misc/eval/scripts"

run() {
    cd $EVAL_SCRIPTS_DIR
    if [[ $1 == "olden" ]]; then
        echo "Performance overhead on Olden:"
        ./olden_perf.py
        echo ""
        echo "Memory overhead on Olden:"
        cd mem
        ./olden_mem.py
    elif [[ $1 == "parson" ]]; then
        echo "Performance overhead on parson:"
        cd $EVAL_SCRIPTS_DIR
        cat ../perf_data/parson/perf.csv
        echo ""
        echo "Memory overhead on parson:"
        cat ../mem_data/parson/mem.csv
    elif [[ $1 == "lzfse" ]]; then
        echo "Performance overhead on lzfse:"
        ./lzfse_perf.py
        echo ""
        echo "Memory overhead on parson:"
        cd mem
        ./lzfse_mem.py
    else
        echo "Unknown benchmark name. Please choose one in "olden", "parson", lzfse".
    fi
}

#
# Entranche of this script
if [[ $# == 0 ]]; then
    echo "Please specify one benchmark: olden, parson, or lzfse"
    exit
else
    run $1
fi

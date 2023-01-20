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
        ./parson_perf.py
        echo ""
        echo "Memory overhead on parson:"
        cd mem
        ./parson_mem.py
    elif [[ $1 == "lzfse" ]]; then
        cd $EVAL_SCRIPTS_DIR
        echo "Performance overhead on lzfse:"
        ./lzfse_perf.py
        echo ""
        echo "Memory overhead on parson:"
        cd mem
        ./lzfse_mem.py
    elif [[ $1 == "thttpd" ]]; then
        cd $EVAL_SCRIPTS_DIR
        echo "Performance overhead on thttpd:"
        ./thttpd_perf.py
        echo ""
        echo "Memory overhead on thttpd:"
        cd mem
        ./thttpd_mem.py
    elif [[ $1 == "curl" ]]; then
        cd $EVAL_SCRIPTS_DIR
        echo "Performance overhead on curl:"
        ./curl_perf.sh
        echo ""
        echo "Memory overhead on curl:"
        cd mem
        ./curl_mem.py
    else
        echo "Unknown benchmark name. Please choose one in "olden", "parson", lzfse".
    fi
}

#
# Entranche of this script
if [[ $# == 0 ]]; then
    echo "Please specify one benchmark: olden, parson, lzfse, thttpd, or curl."
    exit
else
    run $1
fi

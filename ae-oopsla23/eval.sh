#!/usr/bin/env bash

# This scripts invokes scripts to run benchmarks for evaluations.

set -e

ROOT_DIR=`realpath .`
SCRIPTS_DIR="$ROOT_DIR/misc/scripts"
EVAL_SCRIPTS_DIR="$ROOT_DIR/misc/eval/scripts"
CETS_DIR="$ROOT_DIR/cets"


#
# Olden performance evaluation.
#
olden_perf() {
    cd "$EVAL_SCRIPTS_DIR"

    echo "Running baseline Olden benchmarks"
    ./olden_run.sh baseline

    echo "Running Checked C Olden benchmarks"
    ./olden_run.sh checked

    echo "Running CETS Olden benchmarks"
    ./olden_run.sh cets

    # Compute the performance overhead of Checked C and CETS
    ./olden_perf.py
}

#
# Olden memory evaluation.
#
olden_mem() {
    cd "$EVAL_SCRIPTS_DIR/mem"
    echo "Evaluating memory overhead of Checked C and CETS on Olden"

    echo "Running baseline Olden benchmarks"
    ./olden_run.sh baseline

    echo "Running Checked C Olden benchmarks"
    ./olden_run.sh checked

    echo "Running CETS Olden benchmarks"
    ./olden_run.sh cets

    # Compute the memory overhead of Checked C and CETS
    ./olden_mem.py
}

#
# parson eval.
#
parson_eval() {
    echo ""
    cd $EVAL_SCRIPTS_DIR
    echo "Evaluating Checked C's performance overhead on parson"
    ./parson_perf.py

    echo "Evaluating Checked C's memory overhead on parson"
    cd mem
    ./parson_run.sh baseline
    ./parson_run.sh checked
    ./parson_mem.py
}

#
# lzfse eval.
#
lzfse_eval() {
    echo ""
    cd $EVAL_SCRIPTS_DIR
    echo "Evaluating Checked C's performance overhead on lzfse"
    ./lzfse_run.sh baseline
    ./lzfse_run.sh checked

    echo "Evaluating Checked C's memory overhead on lzfse"
    cd mem
    ./lzfse_run.sh baseline
    ./lzfse_run.sh checked

    # Compute results.
    cd ..
    ./lzfse_perf.py
    cd mem
    ./lzfse_mem.py
}

#
# thttpd eval.
#
thttpd_eval() {
    echo ""
    cd "$EVAL_SCRIPTS_DIR"
    echo "Evaluating Checked C performance overhead on thttpd"
    ./thttpd_run.sh baseline
    ./thttpd_run.sh checked

    echo "Evaluating Checked C memory overhead on thttpd"
    cd mem
    ./thttpd_run.sh baseline
    ./thttpd_run.sh checked

    # Compute results
    cd ..
    ./thttpd_perf.py
    cd mem
    ./thttpd_mem.py
}

#
# curl eval.
#
curl_eval() {
    echo ""
    cd $EVAL_SCRIPTS_DIR
    echo "Evaluating Checked C performance overhead on curl"
    ./curl_run.sh baseline
    ./curl_run.sh checked

    echo "Evaluating Checked C memory overhead on curl"
    cd mem
    ./curl_run.sh

    # Compute results.
    cd ..
    ./curl_perf.sh
    cd mem
    ./curl_mem.py
}

#
# Evaluating on all benchmakrs.
#
eval_all() {
    olden_perf
    olden_mem

    parson_eval

    lzfse_eval

    thttpd_eval

    curl_eval
}

#
# Entrance of this script
#
if [[ $# == 0 ]]; then
    eval_all
else
    case $1 in
        "olden")
            olden_perf
            olden_mem
            ;;
        "thttpd")
            thttpd_eval
            ;;
        "parson")
            parson_eval
            ;;
        "lzfse")
            lzfse_eval
            ;;
        "curl")
            curl_eval
            ;;
        *)
            echo "Unknown benchmark names."\
                "Please choose \"olden\", \"thttpd\", \"parson\", \"lzfse\", or \"curl\"."
            ;;
    esac
fi

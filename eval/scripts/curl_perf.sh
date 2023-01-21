#!/usr/bin/env bash

# This script computes the Checked C's performance overhead on curl.

set -e

. common.sh

DATA_DIR="$DATA_DIR/curl"

ITER=20

#
# Compute the average execution time is:
#
perf() {
    cd $DATA_DIR/baseline
    baseline=`grep considered * | cut -d' ' -f7 | paste  -sd+ - | bc`
    baseline=`echo "$baseline/$ITER" | bc`
    cd ../checked
    checked=`grep considered * | cut -d' ' -f7 | paste  -sd+ - | bc`
    checked=`echo "$checked/$ITER" | bc`

    echo "baseline = "$baseline"s, checked = "$checked"s"

    result=`echo "scale=3;$checked/$baseline" | bc`
    echo "Checked C's normalized execution time = $result"
}

perf

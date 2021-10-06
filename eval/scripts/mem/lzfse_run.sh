#!/usr/bin/env bash

#
# This script runs the baseline and Checked C lzfse for the purpose of
# memory consumption measurement.
#
# $1 - "baseline" or "checked"
#
# Note: This scripts runs based on the assumption that lzfse binaries have
# already been built.
#

. common.sh

# Results data directory.
DATA_DIR="$DATA_DIR/lzfse"
# lzfse executable directory.
BASELINE_DIR="$BENCHMARK_DIR/baseline/lzfse-1.0/build"
CHECKED_DIR="$BENCHMARK_DIR/checked/lzfse-1.0/build"
# Directory of the datasets to be compressed/decompressed
INPUT_DIR="$EVAL_DIR/lzfse_dataset"

INPUTS=(
    "dickens"
    "mozilla"
    "mr"
    "nci"
    "ooffice"
    "osdb"
    "reymont"
    "samba"
    "sao"
    "webster"
    "xml"
    "x-ray"
    "enwik9"
)

#
#  Run lzfse and wss.pl.
#
run() {
    # Prepare directories.
    if [[ $1 == "baseline" ]]; then
        lzfse_dir=$BASELINE_DIR
        data_dir="$DATA_DIR/baseline"
    else
        lzfse_dir=$CHECKED_DIR
        data_dir="$DATA_DIR/checked"
    fi
    mkdir -p $data_dir
    rm -rf $data_dir/*
    cd $lzfse_dir

    # Run lzfse and wss to collect memory consumption data.
    for data in ${INPUTS[@]}; do
        # Compress
        echo "Measuring memory consumption for compressing $data"
        input=$INPUT_DIR/$data.encoded
        output="/tmp/compressed"
        ./lzfse -encode -i $input -o $output &\
            $WSS -s 0 $! 0.05 >& $data_dir/"$data"_encode.stat || true

        # Decompress
        echo "Measuring memory consumption for decompressing $data"
        input=$INPUT_DIR/$data.encoded
        output="/tmp/origin"
        ./lzfse -decode -i $input -o $output &\
            $WSS -s 0 $! 0.05 >& $data_dir/"$data"_decode.stat || true
    done
}

#
# Clean temporary data files.
#
clean() {
    rm -f /tmp/compressed /tmp/origin
}

#
# Entrance of this script.
#
run $1

clean

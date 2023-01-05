#!/usr/bin/env bash

#
# This script runs the baseline and the checked lzfse for performance evaluation.
#

. common.sh

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

ITER=20
INPUT_DIR="$EVAL_DIR/lzfse_dataset"
DATA_DIR="$DATA_DIR/lzfse"
BIN_DIR="$BENCHMARKS_DIR"

#
# Initialize the evaluation environment
#
init() {
    # Prepare directories
    if [[ $1 == "baseline" ]]; then
        echo "Run the baseline lzfse"
        DATA_DIR="$DATA_DIR/baseline"
        BIN_DIR="$BIN_DIR/baseline/lzfse-1.0/build"
    else
        echo "Run the checked lzfse"
        DATA_DIR="$DATA_DIR/checked"
        BIN_DIR="$BIN_DIR/checked/lzfse-1.0/build"
    fi

    # Create and clean data directories if needed.
    if [[ ! -d $DATA_DIR ]]; then
        mkdir -p $DATA_DIR
        mkdir "$DATA_DIR/compress"
        mkdir "$DATA_DIR/decompress"
    fi
    rm -rf $DATA_DIR/compress/*
    rm -rf $DATA_DIR/decompress/*

    # Check if the binary executable exists.
    cd $BIN_DIR
    if [[ ! -f "$BIN_DIR/lzfse" ]]; then
        ./cmake-gen.sh
        make
    fi
}

#
# Run lzfse
#
run() {
    if [[ ! -f "lzfse" ]]; then
        echo "Cannot find the executable, please compile it first."
        exit
    fi

    for i in $(seq 1 $ITER); do
        # Compression
        echo "Iteration $i:"
        for input in ${INPUTS[@]}; do
            echo "Compressing $input..."
            result_file=$DATA_DIR/compress/$input.$i
            input="$INPUT_DIR/$input"
            output="/tmp/compressed"
            ./lzfse -v -encode -i "$input" -o $output >& $result_file
            if [[ ! -f "$input.encoded" ]]; then
                mv $output "$input.encoded"
            fi
            rm -f $output
        done
        echo "Done compressing all files."

        # Decomrepssion
        for input in ${INPUTS[@]}; do
            echo "Decompressing $input..."
            result_file=$DATA_DIR/decompress/$input.$i
            input=$INPUT_DIR/$input.encoded
            output="/tmp/origin"
            ./lzfse -v -decode -i "$input" -o $output >& $result_file
            rm -f $output
        done
        echo "Done decompressing all files."
        echo
    done
}

#
# Entrance of this script
#
init $1

run

#!/usr/bin/env bash

#
# This script runs the baseline and the checked lzfse for performance evaluation.
#

set -e

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

#
# Initialize the evaluation environment
#
init() {
    case $1 in
        "baseline"|"checked")
            target=$1
            ;;
        *)
            echo "Unknow target!"
            exit 1
            ;;
    esac

    # Prepare directories
    DATA_DIR="$DATA_DIR/$target"
    BIN_DIR="$BENCHMARKS_DIR/$target/lzfse-1.0/build"

    # Create and clean data directories if needed.
    mkdir -p "$DATA_DIR/compress"
    mkdir -p "$DATA_DIR/decompress"
    rm -rf $DATA_DIR/compress/*
    rm -rf $DATA_DIR/decompress/*

    # Check if the binary executable exists.
    cd $BIN_DIR
    if [[ ! -f "$BIN_DIR/lzfse" ]]; then
        ./cmake-gen.sh
        make
    fi

    if [[ ! -f "lzfse" ]]; then
        echo "Cannot find or make the executable, please compile it first."
        exit 1
    fi
}

#
# Run lzfse
#
run() {
    echo "Run the $target lzfse"

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

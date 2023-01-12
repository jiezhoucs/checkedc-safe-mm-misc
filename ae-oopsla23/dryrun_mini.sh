#!/usr/bin/env bash

#
# This scripts does a quick run on the compilers and benchmarks for testing.
#
# $1: The name of a benchmark: olden, parson, lzfse, thttpd, curl.
#

set -e

ROOT_DIR=`realpath .`
SCRIPTS_DIR="$ROOT_DIR/misc/scripts"
CETS_DIR="$ROOT_DIR/cets"
EVAL_DIR="$ROOT_DIR/misc/eval"
EVAL_SCRIPTS_DIR="$EVAL_DIR/scripts"
BENCHMARKS_DIR="$ROOT_DIR/misc/benchmarks"

#
# Test compiling and running the bisort benchmark of Olden.
#
run_olden() {
    cd "$SCRIPTS_DIR"
    echo "Testing compiling and running baseline bisort..."
    ./olden-baseline.sh clean
    ./olden-baseline.sh bisort

    echo ""
    echo "Testing compiling and running Checked C bisort..."
    ./olden.sh clean
    ./olden.sh bisort

    echo ""
    echo "Testing compiling and running CETS bisort..."
    cd cets
    ./olden.sh clean
    ./olden.sh bisort

    echo ""
}

#
# Test compiling and running parson
#
run_parson() {
    cd "$BENCHMARKS_DIR"
    echo "Testing compiling and running baseline parson..."
    cd baseline/parson
    make clean
    make test

    echo ""
    echo "Testing compiling and running Checked C parson..."
    cd ../../checked/parson
    make clean
    make test

    echo ""
}

#
# Test compiling and running lzfse
#
run_lzfse() {
    cd "$BENCHMARKS_DIR"
    echo "Testing compiling and running baseline lzfse with input dickens..."
    cd baseline/lzfse-1.0/build
    make clean
    make
    ./lzfse -v -encode -i "$EVAL_DIR/lzfse_dataset/dickens" -o /tmp/dickens.encoded
    echo ""

    ./lzfse -v -decode -i /tmp/dickens.encoded -o /tmp/dickens.decoded
    rm /tmp/dickens.*

    echo ""
    echo "Testing compiling and running Checked C lzfse with input dickens..."
    cd "$BENCHMARKS_DIR/checked/lzfse-1.0/build"
    make clean
    make
    ./lzfse -v -encode -i "$EVAL_DIR/lzfse_dataset/dickens" -o /tmp/dickens.encoded
    echo ""

    ./lzfse -v -decode -i /tmp/dickens.encoded -o /tmp/dickens.decoded
    rm /tmp/dickens.*

    echo ""
}

#
# Test running thttpd. Since installing thttpd requires root privilege, we only
# run it here.
#
run_thttpd () {
    cd "$BENCHMARKS_DIR/baseline/thttpd-2.29"
    echo "Testing running baseline thttpd..."
    cd "$SCRIPTS_DIR"
    ./thttpd.sh baseline

    echo "Testing running Checked C thttpd..."
    ./thttpd.sh checked

    echo ""
}

#
# Test running curl.
#
run_curl() {
    cd "$BENCHMARKS_DIR/baseline/curl/tests"
    echo "Test running baseline curl..."
    ./runtests.pl 1 2 3 4 5 6 7 8 9 10

    echo ""
    cd "$BENCHMARKS_DIR/checked/curl/tests"
    echo "Test running baseline curl..."
    ./runtests.pl 1 2 3 4 5 6 7 8 9 10

    echo ""
}


run_all() {
    run_olden
}

#
# Entrance of this script.
#
if [[ $# == 0 ]]; then
    run_all
else
    case $1 in
        "olden")
            run_olden
            ;;
        "thttpd")
            run_thttpd
            ;;
        "parson")
            run_parson
            ;;
        "lzfse")
            run_lzfse
            ;;
        "curl")
            run_curl
            ;;
        *)
            echo "Unknown benchmark. Please choose: olden, parson, lzfse, thttpd, curl."
            exit
    esac
fi

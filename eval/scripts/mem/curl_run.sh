#!/usr/bin/env bash

#
# This script runs selected curl tests and wss.pl to collect memory consumption data.
#

. common.sh

DATA_DIR="$DATA_DIR/curl"

#
# curl tests. Sorted by execution time.
#
TESTS=(
    677     # lib
    1501    # lib
    190
    1238
    1523    # lib
    250
    251
    1086
    1117
)

#
# Run tests and wss
#
run() {
    # Configure
    if [[ $1 == "baseline" ]]; then
        test_dir="$BENCHMARK_DIR/baseline/curl/tests"
        data_dir="$DATA_DIR/baseline"
    else
        test_dir="$BENCHMARK_DIR/checked/curl/tests"
        data_dir="$DATA_DIR/checked"
    fi
    mkdir -p $data_dir
    rm -rf $data_dir/*

    cd $test_dir
    # Run selected tests and collect memory data.
    for test in ${TESTS[@]}; do
        echo "Running test $test"
        pid=
        if [[ $test == 677 || $test == 1501 || $test == 1523 ]]; then
            ./runtests.pl $test >& /dev/null &
            while [[ ! $pid ]]; do
                pid=`pgrep -n -U $UID lib$test`
            done
            $WSS -s -0 $pid 0.1 >& $data_dir/$test.stat || true
        else
            ./runtests.pl $test >& /dev/null &
            while [[ ! $pid ]]; do
                pid=`pgrep -n -U $UID curl`
            done
            $WSS -s -0 $pid 0.1  >& $data_dir/$test.stat || true
        fi

        # JZ: Don't why, but without the next sleep this script would fail to
        # capture the correct pid for wss.
        sleep 1
    done
}

#
# Entrance of this script
#
run $1

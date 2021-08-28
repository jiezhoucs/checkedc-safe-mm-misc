#!/bin/bash

#
# This script runs ab to test the performance of the thttpd server.
#
# $1 - (optional) "baseline". Without which this script will run the checked thttpd.
#

. common.sh

DATA_DIR="$EVAL_DIR/perf_data/thttpd"
BUILD_DIR="$BENCHMARK_BUILD/thttpd"

# arguments for the thttpd server
REQUESTS=10000
CONS=8
HOST=http://127.0.0.1
PORT=8080

# Repeated experiments.
ITERATIONS=20

#
# Clean up existing data files and start the server.
#
init() {
    if [[ $1 == "baseline" ]]; then
        echo "Run the baseline thttpd"
        SERVER_DIR="$BUILD_DIR/baseline"
        DATA_DIR="$DATA_DIR/baseline"
    else
        echo "Run the checked thttpd"
        SERVER_DIR="$BUILD_DIR/checked"
        DATA_DIR="$DATA_DIR/checked"
    fi

    # Create data directories if not exisiting.
    if [[ ! -f $DATA_DIR ]]; then
        mkdir -p $DATA_DIR
    fi

    # Clean exisiting result files.
    rm -rf $DATA_DIR/*

    # start the server
    cd $SERVER_DIR
    # pkill thttpd
    # For some unknown reanson, `pkill thttpd` may kill this script itself.
    if [[ `pidof thttpd` ]]; then
        kill -9 `pidof thttpd`
    fi
    sleep 0.1
    ./sbin/thttpd -p 8080

    # Check if the server started successfully
    if [[ `pgrep "thttpd"` ]]; then
        echo "Starting the thttpd server"
    else
        echo "thttpd failed to start!"
        exit
    fi

    # Wait a while to let the server fully start, otherwise we might get
    # the "apr_socket_recv: Connection reset by peer (104)" error after
    # running ab.
    sleep 1.5
}

#
# Start benchmarking.
#
run() {
    cd "$DATA_DIR"
    for ii in {12..25}; do
        # For each iteration, file size is 2 to the power of i
        i=$(python -c "print (2 ** $ii)")

        echo "Testing size $i with file-$i"
        echo "---------------"

        # Check if the server started successfully
        if [[ ! `pgrep "thttpd"` ]]; then
            echo "thttpd failed to start!"
            exit
        fi

        # Run the test appending the output to the file
        for j in $(seq 1 $ITERATIONS); do
            ab -c $CONS -n $REQUESTS $HOST:$PORT/files/file-$i 2>&1 >> results.$i
        done
    done
}

#
# Collect results
#
collect_results() {
    echo "Starting to collect data (file transfer rate)..."

    # for file sizes from 1,024 bytes to 2^24 = 16,777,216 bytes
    for ii in {12..25}; do
        # Print out the size.
        i=$(python -c "print (2 ** $ii)")

        # Print out the results.
        grep "Transfer rate" results.$i | awk '{printf "%s,", $3}' >> bandwidth.dt

        # Print a newline
        echo >> bandwidth.dt
    done

    echo "Finished collecting data. Result file is $DATA_DIR/bandwdith.dt"
}

#
# Entrance of this script
#
init $1

run

collect_results

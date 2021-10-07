#!/bin/bash

#
# This script runs ab to test the performance of the thttpd server.
#
# $1 - (optional) "baseline". Without which this script will run the checked thttpd.
#

. common.sh

DATA_DIR="$EVAL_DIR/mem_data/thttpd"
BUILD_DIR="$ROOT_DIR/benchmark-build/thttpd"

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
    # `pkill thttpd` may kill this script itself. We use pidof.
    if [[ `pidof thttpd` ]]; then
        kill -9 `pidof thttpd`
    fi
    sleep 0.1
    ./sbin/thttpd -p 8080

    # Check if the server started successfully
    if [[ `pgrep "thttpd"` ]]; then
        echo "thttpd started successfully."
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
# Run ab and wss.
#
run() {
    init $1

    # Start wss.pl.
    pid=`pidof thttpd`
    $WSS -s 0 $pid 0.1 >> $DATA_DIR/mem.stat 2>&1 &

    for ii in {12..25}; do
        # For each iteration, file size is 2 to the power of i
        i=$(python -c "print (2 ** $ii)")

        echo "Transferring file-$i" >> $DATA_DIR/mem.stat
        echo "Transferring file-$i"

        # Run the test appending the output to the file
        ab -c $CONS -n $REQUESTS $HOST:$PORT/files/file-$i >&/dev/null
    done

    # Stop the server.
    echo "Job done. Killing the server."
    pkill thttpd
}

#
# Entrance of this script
#
run $1

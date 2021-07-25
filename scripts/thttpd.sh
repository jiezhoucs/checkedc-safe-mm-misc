#!/usr/bin/env bash

#
# This script starts a local thttpd server and tests it by transfering files.
#

EVAL_DIR=`realpath ../eval`
DATA_DIR=$EVAL_DIR/data/thttpd
BUILD_DIR=$EVAL_DIR/../../benchmark-build/thttpd

ITERS=1000
CONS=8
HOST=http://127.0.0.1
PORT=8080
FILES=$HOST:$PORT/files

#
# Initialization
#
init() {
    # Check if ab is installed in the machine.
    if [[ ! `which ab` ]]; then
        echo "ab not existing. Please first install ab"
        exit
    fi

    # Set up directories.
    if [[ $1 == "baseline" ]]; then
        DATA_DIR=$DATA_DIR/baseline
        BUILD_DIR=$BUILD_DIR/baseline

    else
        DATA_DIR=$DATA_DIR/checked
        BUILD_DIR=$BUILD_DIR/checked
    fi

    # Create data directories if not exisiting.
    if [[ ! -f $DATA_DIR ]]; then
        mkdir -p $DATA_DIR
    fi

    # Clean exisiting result files.
    rm -f $DATA_DIR/*

    # Start the server
    pkill thttpd
    sleep 0.1
    cd $BUILD_DIR
    ./sbin/thttpd -p 8080

    # Check if the server started successfully
    if [[ `pgrep "thttpd"` ]]; then
        echo "thttpd is running ..."
    else
        echo "thttpd start failed"
        exit
    fi

    # Wait a while to let the server fully start, otherwise we might get
    # the "apr_socket_recv: Connection reset by peer (104)" error.
    sleep 1.5
}

#
# Run multiple iterations to collect bandwidth data
#
# run() {
#     # to-do
# }

#
# for debugging purpose
#
debug() {
    echo "Testing ......"
    ab -c $CONS -n $ITERS $FILES/file-1024
}

#
# Entrance of this script
#
init

debug

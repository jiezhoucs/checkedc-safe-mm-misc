#!/usr/bin/env bash

#
# This scripts starts the nginx server and runs ab to transfer files.
#

ROOT_DIR=`realpath ../../../`
ANALYSIS_DIR="$ROOT_DIR/analysis"
NGINX_DIR="$ANALYSIS_DIR/programs/nginx-1.21.3"
SERVER_ROOT_DIR="$NGINX_DIR/build"

# Number of open connections.
CON=8

# Requets
REQUESTS=10000

#
# Time (second) limited to benchmarking.
#
TIMELIMIT=10

HOST=http://127.0.0.1
PORT=8090

#
# Parameters for transferred files
#
START=20
END=25

#
# Configure the server
#
# We need this run configure function every time because the cleaning procedure
# (make clean) of nginx deletes the whole build directory.
#
config() {
    cd $SERVER_ROOT_DIR
    # Set the port number
    port=8`head -36 conf/nginx.conf | tail -1 | cut -d '8' -f2`
    if [[ $port == "80;" ]]; then
        sed -i "36s/80/8090/" conf/nginx.conf
    fi

    # Copy random files here
    if [[ ! -d html/files ]]; then
        cp -r ../../../files html/
    fi
}

#
# Start the nginx server and run ab to transfer files.
#
run() {
    cd $SERVER_ROOT_DIR
    # Start the server. Kill it if it is already started.
    pkill nginx
    # Start the server
    ./sbin/nginx

    # Remove old data file
    rm -f /tmp/analysis_result.txt

    # Check if the served was started successfully.
    if [[ ! `pgrep "nginx"` ]]; then
        echo "Failed to start the server!"
        exit
    fi

    for i in $(seq $START $END); do
        size=`echo 2^$i | bc`
        echo "Fetching file file-$size..."
        ab -c $CON -n $REQUESTS $HOST:$PORT/files/file-$size
    done

    ./sbin/nginx -s quit
}

#
# Entrance of this script.
#
config

run

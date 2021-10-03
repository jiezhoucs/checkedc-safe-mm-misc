#!/usr/bin/env bash

#
# This scripts starts the httpd server and runs ab to transfer files.
#

ROOT_DIR=`realpath ../../../`
ANALYSIS_DIR="$ROOT_DIR/analysis"
HTTPD_DIR="$ANALYSIS_DIR/programs/httpd-2.4.46"

SERVER_ROOT_DIR="$HTTPD_DIR/install"

# Number of open connections.
CON=8

# Requets
REQUESTS=10000

#
# Time (second) limited to benchmarking.
#
TIMELIMIT=10

#
# Host and port
#
HOST=http://127.0.1.1
PORT=8091

#
# Parameters for transferred files
#
START=20
END=25

#
# Set the port number and copy files to the root directory if they do not exit.
#
prepare() {
    cd $SERVER_ROOT_DIR
    # Copy random files to here.
    if [[ ! -d "htdocs/files" ]]; then
        cp -r ../../../files htdocs/
    fi

    # Set the port number
    port=8`head -52 conf/httpd.conf | tail -1 | cut -d '8' -f2`
    if [[ $port == "80" ]]; then
        sed -i "52s/80/8091/" conf/httpd.conf
    fi
}

#
# Start the server and run ab
#
run() {
    cd $SERVER_ROOT_DIR
    # Start the server. Kill it if it is already started.
    pkill nginx
    # Start the server
    ./bin/httpd

    # Remove old data file
    rm -f /tmp/analysis_result.txt

    # Check if the served was started successfully.
    if [[ ! `pgrep "httpd"` ]]; then
        echo "Failed to start the server!"
        exit
    fi

    # Run ab.
    for i in $(seq 20 25); do
        size=`echo 2^$i | bc`
        echo "Fetching file file-$size..."
        ab -c $CON -n $REQUESTS $HOST:$PORT/files/file-$size
    done

    # Gracefully turn off the server.
    ./bin/httpd -k graceful-stop
}

#
# Entrance of this script
#
prepare

run

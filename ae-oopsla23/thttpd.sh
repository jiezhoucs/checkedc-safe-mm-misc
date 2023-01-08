#!/usr/bin/env bash

#
# This script builds, installs, and runs thttpd. Installing it requires the
# root privilege.
#
# $1: "build", "install", or "eval"
#

ROOT_DIR=`realpath .`
SCRIPTS_DIR="$ROOT_DIR/misc/scripts"
EVAL_SCRIPTS_DIR="$ROOT_DIR/misc/eval/scripts"
BASELINE_SRC="$ROOT_DIR/misc/benchmark/baseline/thttpd-2.29"
CHECKED_SRC="$ROOT_DIR/misc/benchmark/checked/thttpd-2.29"

#
# Entrance of this script
#
if [[ $1 == "build" ]]; then
    cd "$SCRIPTS_DIR"
    ./thttpd_build.sh baseline
    ./thttpd_build.sh checked
elif [[ $1 == "install" ]]; then
    cd $BASELINE_SRC
    make install

    cd $CHECKED_SRC
    make install
elif [[ $# == 0 || $1 == "eval" ]]; then
    # First, generate input files for thttpd.
    cd "$SCRIPTS_DIR"
    ./thttpd_file.py

    cd "$EVAL_SCRIPTS_DIR"
    ./thttpd_run.sh baseline
    ./thttpd_run.sh checked
else
    echo "Unknown argument. Please choose "build", "install" (require root), or "eval".
fi


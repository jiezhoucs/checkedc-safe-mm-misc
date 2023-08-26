#!/usr/bin/env bash

#
# This script runs the Olden benchmarks and wss.pl to collect their memory
# consumption data.
#
# $1 - "baseline", "checked", or "cets".
#
# @note We do not use "set -e" because wss may terminate this script when the
#       target application finishes running and so wss fails to capture the memory
#       data of the target's process.
#

. ../common.sh

DATA_DIR="$EVAL_DIR/mem_data/olden"
BENCHMARKS=("${OLDEN_BENCHMARKS[@]}")
CETS_BENCHMARKS=("${OLDEN_CETS_BENCHMARKS[@]}")

#
# The next command prevents `pgrep` getting the pid of `gsd-power` when
# processing the power benchmark. This may happen in machines where gsd-power
# was started by the current user instead of gdm.
#
# @note JZ: I'm confused with why this still happens even if `-n` is used in
# the pgrep command that finds the pid of the power benchmark.
#
function handle_gsd-power {
    pid=$(pgrep gsd-power)
    if [[ ! -z $pid ]]; then
        user=$(ps -o user= -p $pid)
        if [[ $user == $USER ]]; then
            pkill gsd-power
        fi
    fi
}

#
# Run Olden benchmarks, and run wss.pl to collect memory consumption data.
#
main() {
    handle_gsd-power

    cd $MISC_SCRIPTS

    case $1 in
        "baseline"|"checked"|"cets")
            target=$1
            ;;
        *)
            echo "Unknow target!"
            exit 1
            ;;
    esac

    # Prepare script and data directory.
    olden_script="$MISC_SCRIPTS/olden.sh"
    data_dir="$DATA_DIR/$target"
    mkdir -p $data_dir
    rm -rf $data_dir/*

    # Run benchmarks and collect memory data produced by wss.pl.
    for prog in ${BENCHMARKS[@]}; do
        if [[ $1 == "cets" && ! " ${CETS_BENCHMARKS[@]} " =~ " $prog " ]]; then
            echo "Skip $prog for cets"
            continue
        fi

        echo "Measuring memory consumption of $prog"
        pid=
        $olden_script $target $prog &
        # Since it launches llvm-lit to run a benchmark, there might be a brief
        # latency before the benchmark actually starts. We therefore use a loop
        # to catch the pid of the benchmark process.
        while [[ ! $pid ]]; do
            pid=`pgrep -n -U $UID $prog`
        done
        # Collect memory consumption data every 0.1 second.
        $WSS -s 0 $pid 0.1 >& $data_dir/$prog.stat || true
        echo "Finished measuring memory consumption of $prog"
    done
}

#
# Entrance of this script.
#
main $1

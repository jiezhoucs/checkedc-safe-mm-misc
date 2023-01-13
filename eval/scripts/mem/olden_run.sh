#!/usr/bin/env bash

#
# This script runs the Olden benchmarks and wss.pl to collect their memory
# consumption data.
#
# $1 - "baseline", "checked", or "cets".
#

. common.sh

DATA_DIR="$DATA_DIR/olden"

BENCHMARKS=(
    "bh"
    "bisort"
    "em3d"
    "health"
    "mst"
    "perimeter"
    "power"
    "treeadd"
    "tsp"
)

CETS_BENCHMARKS=(
    # "bh"
    "bisort"
    # "em3d"
    "health"
    # "mst"
    "perimeter"
    "power"
    "treeadd"
    "tsp"
)

#
# Run that scripts that run Olden benchmarks, and run wss.pl to collect
# memory consumption data.
#
run() {
    # To prevent `pgrep` getting the pid of `gsd-power` when processing the
    # power benchmark. This may happen in machines where gsd-power was started
    # by the current user instead of gdm.
    # JZ: I'm confused why this still happens even if we have `-n` in the
    # pgrep command  that finds the pid of the power benchmark.
    pkill gsd-power

    cd $SCRIPTS_DIR

    # Prepare script and data directory.
    if [[ $1 == "baseline" ]]; then
        olden_script="./olden-baseline.sh"
        data_dir="$DATA_DIR/baseline"
    elif [[ $1 == "cets" ]]; then
        olden_script="cets/olden.sh"
        data_dir="$DATA_DIR/cets"
    else
        olden_script="./olden.sh"
        data_dir="$DATA_DIR/checked"
    fi
    mkdir -p $data_dir
    rm -rf $data_dir/*

    # Run benchmarks and collect memory data produced by wss.pl.
    for prog in ${BENCHMARKS[@]}; do
        # Skip over benchmarks that CETS cannot handle.
        if [[ $1 == "cets" &&
            ($prog == "bh" || $prog == "em3d" || $prog == "mst") ]]; then
            continue
        fi

        echo "Measuring memory consumption of $prog"
        pid=
        $olden_script $prog &
        # Since it launches llvm-lit to run a benchmark, there might be a little
        # latency before the benchmark is really started. We therefore use
        # a loop to catch the pid of the benchmark process.
        while [[ ! $pid ]]; do
            pid=`pgrep -n -U $UID $prog`
        done
        # Collect memory consumption data every 0.1 second.
        $WSS_DIR/wss.pl -s 0 $pid 0.1 >& $data_dir/$prog.stat || true
    done
}

#
# Entrance of this script.
#
run $1

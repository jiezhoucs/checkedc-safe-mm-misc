#!/usr/bin/env python3

#
# This script runs olden benchmarks multiple times and collect data.
#

import numpy as np
import subprocess as sp
import os
import json

ROOT_DIR = os.path.abspath(os.getcwd() + "/../..")
DATA_DIR = ROOT_DIR + "/eval/perf_data/olden"
SCRIPT_DIR = ROOT_DIR + "/scripts"

ITERATION = 10

benchmarks = [
    "bh",
    "bisort",
    "em3d",
    "health",
    "mst",
    "perimeter",
    "power",
    "treeadd",
    "tsp",
]

exec_time_checked = { }
exec_time_origin = { }

#
# Function: run()
# This function runs the original or the checked version of all benchmarks
# ITERATION number of times and computes the arithmetic mean of the execution
# time.
#
def run(version):
    global exec_time_checked, exec_time_origin
    if version == "origin":
        data = DATA_DIR + "/origin"
        script = SCRIPT_DIR + "/olden-origin.sh"
        exec_time = exec_time_origin
    else:
        data = DATA_DIR + "/checkedc"
        script = SCRIPT_DIR + "/olden.sh"
        exec_time = exec_time_checked

    # Init
    os.chdir(SCRIPT_DIR)
    sp.run([script, "clean"])
    for benchmark in benchmarks:
        exec_time[benchmark] = 0

    # Run the olden script and collect executime time data
    for i in range(ITERATION):
        for benchmark in benchmarks:
            os.chdir(SCRIPT_DIR)
            sp.run([script, benchmark])
            os.chdir(data)
            grep = sp.Popen(("grep","exec_time", benchmark + ".json"), stdout=sp.PIPE)
            time = sp.check_output(("cut", "-d", ":", "-f2"), stdin=grep.stdout)
            exec_time[benchmark] += float(time.decode('utf-8').rstrip()[1:-1])

    # Compute the average execution time.
    for benchmark in benchmarks:
        exec_time[benchmark] /= ITERATION


#
# Print out the results:
# - slowdown (x) : checked / origin
# - geo. mean of slowdown
#
def print_result():
    print(exec_time_origin)
    print(exec_time_checked)

    slowdown = []
    for benchmark in benchmarks:
        time_orign, time_checked = exec_time_origin[benchmark], exec_time_checked[benchmark]
        slowdown += [time_checked / time_orign]
        print("%.2f " % slowdown[-1])

    print("Geometric mean of all benchmarks: ")
    print(np.array(slowdown).prod() ** (1.0/len(slowdown)))


#
# Entrance of this script
#
def main():
    run("origin")
    run("checked")

    print_result()

if __name__ == "__main__":
    main()

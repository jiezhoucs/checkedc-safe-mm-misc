#!/usr/bin/env python3

#
# This script runs CETS-compiled olden benchmarksmultiple times and collect data.
#

import numpy as np
import subprocess as sp
import os
import json
import csv

ROOT_DIR = os.path.abspath(os.getcwd() + "/../..")
DATA_DIR = ROOT_DIR + "/eval/perf_data/olden"
SCRIPT_DIR = ROOT_DIR + "/scripts"

ITERATION = 10

benchmarks = [
    # "bh",
    "bisort",
    # "em3d",
    "health",
    # "mst",
    "perimeter",
    "power",
    "treeadd",
    "tsp",
]

exec_time_checked = { }
exec_time_baseline = { }

#
# Function: run()
# This function runs the baseline or the cets version of all benchmarks
# ITERATION number of times and computes the geo. mean of the execution time.
#
def run(version):
    global exec_time_checked, exec_time_baseline
    if version == "baseline":
        data = DATA_DIR + "/baseline"
        script = SCRIPT_DIR + "/olden-baseline.sh"
        exec_time = exec_time_baseline
    else:
        data = DATA_DIR + "/cets"
        script = SCRIPT_DIR + "/cets/olden.sh"
        exec_time = exec_time_checked

    # Init
    os.chdir(SCRIPT_DIR)
    if version != "baseline":
        os.chdir("cets")
        sp.run([script, "clean"])
    for prog in benchmarks:
        exec_time[prog] = 0

    # Run the olden script and collect executime time data
    for i in range(ITERATION):
        for prog in benchmarks:
            os.chdir(SCRIPT_DIR)
            if version != "baseline":
                os.chdir("cets")
            sp.run([script, prog])
            os.chdir(data)
            grep = sp.Popen(("grep","exec_time", prog + ".json"), stdout=sp.PIPE)
            time = sp.check_output(("cut", "-d", ":", "-f2"), stdin=grep.stdout)
            exec_time[prog] += float(time.decode('utf-8').rstrip()[1:-1])

    # Compute the average execution time.
    for prog in benchmarks:
        exec_time[prog] /= ITERATION


#
# Print out the results and write them to a file.
#
def write_result():
    print("Execution time of baseline programs:")
    print(exec_time_baseline)
    print("Execution time of CETS programs:")
    print(exec_time_checked)

    normalized = []       # normalized execution time of the checked programs
    prog_normalized = {}  # prog:normalized
    for prog in benchmarks:
        time_orign, time_checked = exec_time_baseline[prog], exec_time_checked[prog]
        normalized += [time_checked / time_orign]
        prog_normalized[prog] = round(normalized[-1], 2)
        print("%.2f " % normalized[-1])

    print("Geo. mean of Olden benchmarks: ", end='')
    print(np.array(normalized).prod() ** (1.0/len(normalized)))

    # Write the result to a CVS file.
    with open(DATA_DIR + "/cets_perf.csv", "w") as perf_csv:
        writer = csv.writer(perf_csv)
        header = ["program", "baseline(s)", "checked(s)", "normalized(x)"]
        writer.writerow(header)

        for prog in benchmarks:
            row = [prog]
            row += [exec_time_baseline[prog]]
            row += [exec_time_checked[prog]]
            row += [prog_normalized[prog]]
            writer.writerow(row)

#
# Entrance of this script
#
def main():
    run("baseline")
    run("cets")

    write_result()

if __name__ == "__main__":
    main()

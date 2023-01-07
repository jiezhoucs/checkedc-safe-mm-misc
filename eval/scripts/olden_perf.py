#!/usr/bin/env python3

#
# This script collects Olden performance data.
#

import numpy as np
import os
import json
import csv

ROOT_DIR = os.path.abspath(os.getcwd() + "/../..")
DATA_DIR = ROOT_DIR + "/eval/perf_data/olden/"

ITERATION = 20

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

CETS_SKIPPED = ["bh", "em3d", "mst"]

exec_time_baseline, exec_time_checked, exec_time_cets = {}, {}, {}
normalized_checked, normalized_cets = {}, {}


#
# Collect performance data.
#
def collect_data():
    global exec_time_baseline, exec_time_checked, exec_time_cets
    global normalized_checked, normalized_cets
    # Initialize the execution time
    for prog in benchmarks:
        exec_time_baseline[prog], exec_time_checked[prog] = 0, 0
        if prog not in CETS_SKIPPED:
            exec_time_cets[prog] = 0

    # Use Python's json module to extract the execution time.
    for prog in benchmarks:
        for i in range(1, ITERATION + 1):
            data = json.load(open(DATA_DIR + "baseline/" + prog + "." + str(i) + ".json"))
            exec_time_baseline[prog] += float(data['tests'][0]['metrics']['exec_time'])
            data = json.load(open(DATA_DIR + "checked/" + prog + "." + str(i) + ".json"))
            exec_time_checked[prog] += float(data['tests'][0]['metrics']['exec_time'])
            if prog not in CETS_SKIPPED:
                data = json.load(open(DATA_DIR + "cets/" + prog + "." + str(i) + ".json"))
                exec_time_cets[prog] += float(data['tests'][0]['metrics']['exec_time'])

    # Calculate the arithemetic mean of execution time.
    for prog in benchmarks:
        exec_time_baseline[prog] /= ITERATION
        exec_time_checked[prog] /= ITERATION
        normalized_checked[prog] = exec_time_checked[prog] / exec_time_baseline[prog]
        if prog not in CETS_SKIPPED:
            exec_time_cets[prog] /= ITERATION
            normalized_cets[prog] = exec_time_cets[prog] / exec_time_baseline[prog]

#
# Write the result to three files:
#  - checked.csv
#  - cets.csv
#  - perf.csv (containing both checked and cets data)
#
def write_result():

    # Write Checked C's result to a csv file
    with open(DATA_DIR + "/checked.csv", "w") as checked_csv:
        writer = csv.writer(checked_csv)
        header = ["program", "baseline(s)", "checked(s)", "normalized(x)", "overhead(%)"]
        writer.writerow(header)

        for prog in benchmarks:
            row = [prog]
            row += [round(exec_time_baseline[prog], 2)]
            row += [round(exec_time_checked[prog], 2)]
            row += [round(normalized_checked[prog], 3)]
            row += [round((row[-1] - 1) * 100, 1)]
            writer.writerow(row)
        # Write the geo.mean
        norm, norm_checked_cets = [], []
        for prog in benchmarks:
            norm += [normalized_checked[prog]]
            if prog not in CETS_SKIPPED:
                norm_checked_cets += [norm[-1]]
        geomean_checked = round(np.array(norm).prod() ** (1.0 / len(benchmarks)), 3)
        geomean_checked_cets = round(np.array(norm_checked_cets).prod() ** (1.0 / len(norm_checked_cets)), 3)
        min_checked, max_checked = min(norm), max(norm)
        row = ["Geomean", '', '', geomean_checked, round((geomean_checked - 1) * 100, 1)]
        writer.writerow(row)

    # Write CETS's results to a CSV file
    with open(DATA_DIR + "/cets.csv", "w") as cets_csv:
        writer = csv.writer(cets_csv)
        header = ["program", "baseline(s)", "cets(s)", "normalized(x)", "overhead(%)"]
        writer.writerow(header)

        for prog in benchmarks:
            row = [prog]
            if prog in CETS_SKIPPED:
                row += ['', '', '', '']
            else:
                row += [round(exec_time_baseline[prog], 2)]
                row += [round(exec_time_cets[prog], 2)]
                row += [round(normalized_cets[prog], 3)]
                row += [round((row[-1] - 1) * 100, 1)]
            writer.writerow(row)
        normalized = []
        for prog in benchmarks:
            if prog not in CETS_SKIPPED:
                normalized += [normalized_cets[prog]]
        geomean_cets = round(np.array(normalized).prod() ** (1.0 / len(normalized_cets)), 3)
        min_cets, max_cets = min(normalized), max(normalized)
        row = ["Geomean", '', '', geomean_cets, round((geomean_cets - 1) * 100, 1)]
        writer.writerow(row)

    # Write the summarized Checked C vs. CETS data to a csv file
    with open(DATA_DIR + "/perf.csv", "w") as perf_csv:
        writer = csv.writer(perf_csv)
        header = ["program", "baseline(s)", "checked(s)", "normalized_checked(x)",\
                "cets(s)", "normalized_cets(x)"]
        writer.writerow(header)

        for prog in benchmarks:
            row = [prog]
            row += [round(exec_time_baseline[prog], 2)]
            row += [round(exec_time_checked[prog], 2)]
            row += [round(normalized_checked[prog], 3)]
            if prog not in CETS_SKIPPED:
                row += [round(exec_time_cets[prog], 2)]
                row += [round(normalized_cets[prog], 3)]
            else:
                row += ["", ""]
            writer.writerow(row)
        row = ['Geomean', '', '', geomean_checked, '', geomean_cets]
        writer.writerow(row)

    # Print detailed data
    print_normalized(normalized_checked, "checked")
    # Print summarized results.
    print("Checked C's summarized performance overhead:")
    print("Min = " + str(round((min_checked - 1) * 100, 1)) + "%")
    print("Max = " + str(round((max_checked - 1) * 100, 1)) + "%")
    print("Geomean = " + str(round((geomean_checked - 1) * 100, 1)) + "%")
    # print("CETS-only Geomean = " + str(round((geomean_checked_cets - 1) * 100, 1)) + "%")

    print("")
    print_normalized(normalized_cets, "cets")
    print("CETS' summarized performance overhead:")
    print("Min = " + str(round((min_cets - 1) * 100, 1)) + "%")
    print("Max = " + str(round((max_cets - 1) * 100, 1)) + "%")
    print("Geomean " + str(round((geomean_cets - 1) * 100, 1)) + "%")

#
# Print the normalized execution time
#
def print_normalized(data, compiler):
    if compiler == "checked":
        print("Checked C's normalized execution time:")
    else:
        print("CETS's normalized execution time:")

    for prog in benchmarks:
        if compiler == "cets" and prog in CETS_SKIPPED:
            continue
        print(prog + ": " + str(round(data[prog], 2)))
    print("")


#
# Entrance of this script
#
def main():
    collect_data()

    write_result()

if __name__ == "__main__":
    main()

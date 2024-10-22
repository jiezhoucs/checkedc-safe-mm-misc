#!/usr/bin/env python3

#
# This script reads in Olden benchmarks' memory consumption data and calculates
# the memory overhead introduced by Checked C and CETS.
#

import numpy as np
import csv
import sys
from pathlib import Path
sys.path.append(str(Path(__file__).parent.resolve() / ".."))
from evallib import *

DATA_DIR = EVAL_DIR / "mem_data/olden"

BENCHMARKS = [
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

mem_data = {
    compiler : {
        metric : {} for metric in ["rss", "rss_max", "wss", "wss_max"]
    } for compiler in {"baseline", "checked", "cets"}
}

def Int(num):
    ''' Round a float number to its nearest integer '''
    return int(round(num, 0))

#
# Collect data in the output files from wss.pl, and compute the average and
# max RSS and WSS.
#
# @param setting: "baseline", "checked", or "cets"
#
def collect_data(setting):
    for prog in BENCHMARKS:
        if setting == "cets" and prog in CETS_SKIPPED:
            continue

        data_file = open(DATA_DIR / f"{setting}/{prog}.stat")
        # Skip the headings and the last ERROR reporting line.
        data = data_file.readlines()[2:-1]
        rss, wss = [], []
        for line in data:
            line = line.split()
            rss += [round(float(line[1]), 2)]
            wss += [round(float(line[3]), 2)]
        # The last line of some data files have a 0 RSS. This should be because
        # when wss is checking the memory usage of a process, it has finished
        # its main job and OS just reclaims its memory but has not killed it yet.
        # We should skip this line.
        if rss[-1] == 0:
            rss = rss[:-1]
        if wss[-1] == 0:
            wss = wss[:-1]
        # Exclude data for the first 10% of execution time.
        start, end = int(len(rss) * 0.05), len(rss) - 1
        rss, wss = rss[start:end], wss[start: end]
        # Get the maximum RSS
        mem_data[setting]["rss_max"][prog] = round(max(rss), 2)
        mem_data[setting]["wss_max"][prog] = round(max(wss), 2)
        # Compute the average RSS and WSS.
        mem_data[setting]["rss"][prog] = round(np.mean(rss), 2)
        mem_data[setting]["wss"][prog] = round(np.mean(wss), 2)

#
# Write results to a CSV file.
#
def write_result():
    with open(DATA_DIR / "mem.csv", "w") as mem_csv:
        writer = csv.writer(mem_csv)
        header = ["program", "baseline_rss (MB)", "checked_rss (x)", "cets_rss (x)",\
                "baseline_wss (MB)", "checked_wss (x)", "cets_wss (x)"]
        writer.writerow(header)

        checked_rss_norm, cets_rss_norm = [], []
        checked_wss_norm, cets_wss_norm = [], []
        # The next two lists are Checked C data only including CETS-compiled programs.
        checked_rss_cets_norm, checked_wss_cets_norm = [], []
        for prog in BENCHMARKS:
            row = [prog]
            # Add rss data
            baseline_rss_max = mem_data["baseline"]["rss_max"][prog]
            checked_rss_max = mem_data["checked"]["rss_max"][prog]
            normalized = round(checked_rss_max / baseline_rss_max, 2)
            checked_rss_norm += [normalized]
            # Add baseline memory consumption (MB)
            row += [Int(baseline_rss_max)]
            # Add normalized Checked C memory consumption (X)
            row += [normalized]
            if prog in CETS_SKIPPED:
                row += [""]
            else:
                cets_rss_max = mem_data["cets"]["rss_max"][prog]
                normalized = round(cets_rss_max / baseline_rss_max, 2)
                cets_rss_norm += [normalized]
                # Add normalized CETS memory consumption (X)
                row += [normalized]
                checked_rss_cets_norm += [checked_rss_norm[-1]]

            # Add wss data
            baseline_wss_max = mem_data["baseline"]["wss_max"][prog]
            checked_wss_max = mem_data["checked"]["wss_max"][prog]
            normalized = round(checked_wss_max / baseline_wss_max, 2)
            checked_wss_norm += [normalized]
            # Add baseline memory consumption (MB)
            row += [Int(baseline_wss_max)]
            # Add normalized Checked C memory consumption (X)
            row += [normalized]
            if prog in CETS_SKIPPED:
                row += [""]
            else:
                cets_wss_max = mem_data["cets"]["wss_max"][prog]
                normalized = round(cets_rss_max / baseline_wss_max, 2)
                cets_wss_norm += [normalized]
                # Add normalized CETS memory consumption (X)
                row += [normalized]
                checked_wss_cets_norm += [checked_wss_norm[-1]]
            writer.writerow(row)

        # Compute the geomean of Checked C's and CETS' RSS and WSS.
        checked_len, cets_len = len(BENCHMARKS), len(BENCHMARKS) - len(CETS_SKIPPED)
        checked_rss_geomean = round(np.array(checked_rss_norm).prod() ** (1.0 / checked_len), 2)
        cets_rss_geomean = round(np.array(cets_rss_norm).prod() ** (1.0 / cets_len), 2)
        checked_wss_geomean = round(np.array(checked_wss_norm).prod() ** (1.0 / checked_len), 2)
        cets_wss_geomean = round(np.array(cets_wss_norm).prod() ** (1.0 / cets_len), 2)
        checked_rss_cets_geomean = round(np.array(checked_rss_cets_norm).prod() ** (1.0 / cets_len), 2)
        checked_wss_cets_geomean = round(np.array(checked_wss_cets_norm).prod() ** (1.0 / cets_len), 2)

        row = ["geomean", "", checked_rss_geomean, cets_rss_geomean,\
                "", checked_wss_geomean, cets_wss_geomean]
        writer.writerow(row)


        # Print the summarized data.

        print("Checked C RSS overhead:")
        print("Min = " + to_percent(min(checked_rss_norm)))
        print("Max = " + to_percent(max(checked_rss_norm)))
        print("Geomean = " + to_percent(checked_rss_geomean))
        print("")
        print("CETS RSS overhead:")
        print("Min = " + to_percent(min(cets_rss_norm)))
        print("Max = " + to_percent(max(cets_rss_norm)))
        print("Geomean: " + to_percent(cets_rss_geomean))

        print("")
        print("Checked C WSS overhead:")
        print("Min = " + to_percent(min(checked_wss_norm)))
        print("Max = " + to_percent(max(checked_wss_norm)))
        print("Geomean = " + to_percent(checked_wss_geomean))
        print("")
        print("CETS WSS overhead:")
        print("Min = " + to_percent(min(cets_wss_norm)))
        print("Max = " + to_percent(max(cets_wss_norm)))
        print("Geomean: " + to_percent(cets_wss_geomean))

        # print("Checked C (CETS prog only) RSS Geomean: " + str(checked_rss_cets_geomean))
        # print("Checked C (CETS prog only) WSS Geomean: " + str(checked_wss_cets_geomean))

        # Close result file.
        mem_csv.close()

def to_percent(num):
    return str(int((num - 1) * 100)) + "%"

#
# Entrance of this script
#
if __name__ == "__main__":
    # Collect all the raw data generated by wss.
    collect_data("baseline")
    collect_data("checked")
    collect_data("cets")

    # Write results to a csv file
    write_result()
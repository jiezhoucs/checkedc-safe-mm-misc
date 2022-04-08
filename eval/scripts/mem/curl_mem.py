#!/usr/bin/env python3

'''
This script computes the memory overhead of curl.
'''

import numpy as np
import os
import csv

ROOT_DIR = os.path.abspath(os.getcwd() + "/../../../")
DATA_DIR = ROOT_DIR + "/eval/mem_data/curl/"

data_files = [
    "677",
    "1501",
    "190",
    "1238",
    "1523",
    "250",
    "251",
    "1086",
    "1117",
]

#
# Memory consumption data
#
mem_data = {
    "baseline" : {
        "rss" : { }, "rss_max" : { },
        "wss" : { }, "wss_max" : { },
    },
    "checked" : {
        "rss" : { }, "rss_max" : { },
        "wss" : { }, "wss_max" : { },
    },
}

#
# Collect data from the raw datafiles.
#
def collect_data(setting):
    for data_name in data_files:
        data_file = open(DATA_DIR + setting + "/" + data_name + ".stat")
        # Skip the heading and the last ERROR line.
        data = data_file.readlines()[2:-1]
        rss, wss = [], []
        for line in data:
            line = line.split()
            rss_one, wss_one = round(float(line[1]), 2), round(float(line[3]), 2)
            rss += [rss_one]
            wss += [wss_one]

        rss_max, wss_max = round(max(rss), 2), round(max(wss), 2)
        # Compute the max RSS and WSS.
        mem_data[setting]["rss_max"][data_name] = rss_max
        mem_data[setting]["wss_max"][data_name] = wss_max
        # Compute the average RSS and WSS.
        mem_data[setting]["rss"][data_name] = round(np.mean(rss), 2)
        mem_data[setting]["wss"][data_name] = round(np.mean(wss), 2)


#
# Write results to a CSV file
#
def write_result():
    with open(DATA_DIR + "mem.csv", "w") as mem_csv:
        writer = csv.writer(mem_csv)
        header = ["test", "baseline_rss (MB)", "checked_rss (x)",\
                "baseline_wss (MB)", "checked_wss (x)"]
        writer.writerow(header)

        checked_rss_norm, checked_wss_norm = [], []
        for data_name in data_files:
            row = [data_name]
            # rss data
            baseline_rss_max = mem_data["baseline"]["rss_max"][data_name]
            checked_rss_max = mem_data["checked"]["rss_max"][data_name]
            normalized = round(checked_rss_max / baseline_rss_max, 3)
            checked_rss_norm += [normalized]
            row += [baseline_rss_max]
            row += [normalized]

            # wss data
            baseline_wss = mem_data["baseline"]["wss"][data_name]
            checked_wss = mem_data["checked"]["wss"][data_name]
            normalized = round(checked_wss / baseline_wss, 3)
            checked_wss_norm += [normalized]
            row += [baseline_wss]
            row += [normalized]
            writer.writerow(row)

        # compute the geomean of Checked C's RSS and WSS
        data_num = len(checked_rss_norm)
        checked_rss_geomean = round(np.array(checked_rss_norm).prod() ** (1.0 / data_num), 3)
        checked_wss_geomean = round(np.array(checked_wss_norm).prod() ** (1.0 / data_num), 3)

        row = ["geomean" , "", checked_rss_geomean, "", checked_wss_geomean]
        writer.writerow(row)

        # Print the summarized data
        print("RSS: ")
        print("Min = " + str(min(checked_rss_norm)) + ", ", end="")
        print("Max = " + str(max(checked_rss_norm)) + ", ", end="")
        print("Geomean: " + str(checked_rss_geomean))
        print("WSS: ")
        print("Min = " + str(min(checked_wss_norm)) + ", ", end="")
        print("Max = " + str(max(checked_wss_norm)) + ", ", end="")
        print("Geomean: " + str(checked_wss_geomean))


#
# Entrance of this script
#

if __name__ == "__main__":
    # Collect data
    collect_data("baseline")
    collect_data("checked")

    # Write result to a csv file
    write_result()

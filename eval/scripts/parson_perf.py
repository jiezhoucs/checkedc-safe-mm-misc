#!/usr/bin/env python3

'''
This script computes and prints the performance result of Checked C on parson.
'''
import numpy as np
import os
import sys
import csv

ROOT_DIR = os.path.abspath(os.getcwd() + "/../../")
DATA_DIR = ROOT_DIR + "/eval/perf_data/parson/"

ITERATION = 20

MONGODB_JSON= [
    "countries-small",
    "profiles",
    "covers",
    "books",
    "albums",
    "restaurant",
    "countries-big",
    "zips",
    "images",
    "city_inspections",
    "companies",
    "trades",
]
CITYLOTS_JSON = "citylots"

#
# Read in the performance csv files and print the summarized results.
#
def print_result():
    exe_time_baseline = { }       # {file:avg_exe_time}
    exe_time_checked = { }

    # Read in raw data files.
    # Collect data
    for version in ["baseline", "checked"]:
        data_dir = DATA_DIR + version + "/"
        exe_time = { }
        for i in range(ITERATION):
            with open(data_dir + "result." + str(i + 1) + ".csv", "r") as data:
                reader = csv.DictReader(data)
                for row in reader:
                    JSON_file, run_time = row["file_name"], float(row["exe_time"])
                    if JSON_file not in exe_time:
                        exe_time[JSON_file] = run_time
                    else:
                        exe_time[JSON_file] += run_time

        for JSON_file in MONGODB_JSON:
            exe_time[JSON_file] /= ITERATION
        exe_time[CITYLOTS_JSON] /= ITERATION
        if version == "baseline":
            exe_time_baseline = exe_time
        else:
            exe_time_checked = exe_time

    exe_normalized = { }
    for input_file in exe_time_baseline.keys():
        exe_normalized[input_file] = exe_time_checked[input_file] / exe_time_baseline[input_file]

    print("Checked C's normalized execution time of parson:")
    normalized = []
    for input_file in exe_normalized:
        normalized += [round(exe_normalized[input_file], 2)]
        print(input_file + ": " + str(normalized[-1]))
    geomean = round(np.array(normalized).prod() ** (1.0 / len(normalized)), 3)

    print()
    print("Min = " + str(round((min(normalized) - 1) * 100, 2)) + "%")
    print("Max = " + str(round((max(normalized) - 1) * 100, 2)) + "%")
    print("Geomean = " + str(round((geomean - 1) * 100, 2)) + "%")



#
# Entrance of this script
#
def main():
    print_result()

if __name__ == "__main__":
    main()

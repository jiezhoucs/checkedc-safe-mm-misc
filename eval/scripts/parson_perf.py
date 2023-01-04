#!/usr/bin/env python3

#
# This scripts runs parson and computes the performance overhead.
#

import numpy as np
import subprocess as sp
import os
import csv
import time
import glob

ROOT_DIR = os.path.abspath(os.getcwd() + "/../../")
DATA_DIR = ROOT_DIR + "/eval/perf_data/parson/"
BASELINE_PARSON_DIR = ROOT_DIR + "/benchmarks/baseline/parson/"
CHECKED_PARSON_DIR = ROOT_DIR + "/benchmarks/checked/parson/"

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

FILE_SIZE = {
    "countries-small"  : "328 KB",
    "profiles"         : "454 KB",
    "covers"           : "470 KB",
    "books"            : "524 KB",
    "albums"           : "634 KB",
    "restaurant"       : "666 KB",
    "countries-big"    : "2.2 MB",
    "zips"             : "3.0 MB",
    "images"           : "9.2 MB",
    "city_inspections" : "23 MB",
    "companies"        : "75 MB",
    "trades"           : "232 MB",
    "citylots"         : "181 MB"
}

exe_time_baseline = { }       # {file:avg_exe_time}
exe_time_checked = { }
exe_times_baseline = { }      # file:[exe_time_in_each_run]
exe_times_checked = { }

#
# Compile parson if not exists.
#
def compile_parson():
    if not os.path.exists(BASELINE_PARSON_DIR + "eval"):
        os.chdir(BASELINE_PARSON_DIR)
        sp.run(["make", "eval"])

    if not os.path.exists(CHECKED_PARSON_DIR + "eval"):
        os.chdir(BASELINE_PARSON_DIR)
        sp.run(["make", "eval"])

#
# Main body of this script.
#
# Run parson ITERATION number of times and put execution time to a dictionary.
#
def run(version):
    global exe_time_baseline, exe_time_checked, exe_times_baseline, exe_times_checked
    if version == "baseline":
        exe_time = exe_time_baseline
        exe_times = exe_times_baseline
        parson_dir = BASELINE_PARSON_DIR
    else:
        exe_time = exe_time_checked
        exe_times = exe_times_checked
        parson_dir = CHECKED_PARSON_DIR

    data_dir = DATA_DIR + version + "/"
    data_path = data_dir + "result.csv"

    # Remove old data files
    for data_file in glob.glob(data_dir + "*"):
        os.remove(data_file)

    # First processing all the MongoDB JSON files
    for i in range(ITERATION):
        os.chdir(parson_dir)
        # Execute for each JSON file.
        for file_name in MONGODB_JSON:
            sp.run([parson_dir + "eval", file_name])
            # time.sleep(1)
            if os.path.isfile(data_path) == None:
                sys.exit("cannot find performance result file")

        # rename the result file
        data_path_new = data_dir + "result." + str(i + 1) + ".csv"
        os.rename(data_path, data_path_new)

    # Then processing the citylots JSON file.
    for i in range(ITERATION):
        sp.run([parson_dir + "eval", CITYLOTS_JSON])
        if os.path.isfile(data_path) == None:
            sys.exit("cannot find performance result file")

        # Move the data to the result file that contains the MongoDB data
        result_file = open(data_dir + "result." + str(i + 1) + ".csv", 'a')
        result_file.write(open(data_path, 'r').readlines()[1])

    # Collect data
    for i in range(ITERATION):
        with open(data_dir + "result." + str(i + 1) + ".csv", "r") as data:
            reader = csv.DictReader(data)
            for row in reader:
                JSON_file, run_time = row["file_name"], float(row["exe_time"])
                if JSON_file not in exe_time:
                    exe_time[JSON_file] = run_time
                    exe_times[JSON_file] = [run_time]
                else:
                    exe_time[JSON_file] += run_time
                    exe_times[JSON_file] += [run_time]

    for JSON_file in MONGODB_JSON:
        exe_time[JSON_file] /= ITERATION
    exe_time[CITYLOTS_JSON] /= ITERATION

    # remove used data file
    if os.path.isfile(data_path):
        os.remove(data_path)

#
# Write results to a csv file
#
def write_result():
    JSON_FILES = MONGODB_JSON + [CITYLOTS_JSON]
    print("Baseline execution time for each JSON file:")
    print(exe_time_baseline)
    print("Checked C execution time for each JSON file:")
    print(exe_time_checked)

    print("Baseline execution time coefficient of variation:")
    CV = []
    for JSON_file in JSON_FILES:
        CV += [round(np.std(exe_times_baseline[JSON_file]) / exe_time_baseline[JSON_file], 3)]
    print(CV)
    print("Checked C execution time coefficient of variation:")
    CV = []
    for JSON_file in JSON_FILES:
        CV += [round(np.std(exe_times_checked[JSON_file]) / exe_time_checked[JSON_file], 3)]
    print(CV)

    normalized = []         # Normalized execution time of checked programs
    file_normalized = { }
    for JSON_file in JSON_FILES:
        time_baseline, time_checked = exe_time_baseline[JSON_file], exe_time_checked[JSON_file]
        normalized += [round(time_checked / time_baseline, 3)]
        file_normalized[JSON_file] = round(normalized[-1], 3)

    print("Normalized execution time for each file ", end='')
    print(normalized)
    print("Geo. mean of all JSON data files: ", end='')
    geomean = round(np.array(normalized).prod() ** (1.0 / len(normalized)), 3)
    print(geomean)

    # Write the result to a csv file.
    with open(DATA_DIR + "perf.csv", "w") as perf_csv:
        writer = csv.writer(perf_csv)
        header = ["json_file", "normalized(x)"]
        writer.writerow(header)

        for JSON_file in JSON_FILES:
            name = JSON_file if JSON_file != "city_inspections" else "city\\\\\_insepctions"
            row = [name + " (" + FILE_SIZE[JSON_file] + ")"]
            row += [file_normalized[JSON_file]]
            writer.writerow(row)

        # Write the geo. mean
        row = ["Geomean", geomean]
        writer.writerow(row)

#
# Entrance of this script
#
def main():
    compile_parson()

    run("baseline")
    run("checked")

    write_result()

if __name__ == "__main__":
    main()

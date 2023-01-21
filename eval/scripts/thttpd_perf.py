#!/usr/bin/python3

'''
This script processes the outputs from running the ab HTTP benchmarking tool.
It computes the arithmetic means and std deviations of both baseline and
checked thttpd. It also computes the overhead incurred by Checked C.
'''

import sys
import os
import numpy as np
import csv

DATA_DIR = os.path.abspath(os.getcwd() + "/../..") + "/eval/perf_data/thttpd"

def compute():
    # run a script to get raw data files
    os.chdir(DATA_DIR)

    # open data file
    bw_baseline_file = open("baseline/bandwidth.dt", "r").readlines()
    bw_checked_file = open("checked/bandwidth.dt", "r").readlines()

    if len(bw_baseline_file) != len(bw_checked_file):
        print("ERROR: the baseline and the checked bandwidth data files \
                have different lines of code")
        return

    perf_file = open("perf.csv", "w")
    data_writer = csv.writer(perf_file)
    header = ["file_size (Kb)", "baseline (Mb/s)", "std_dev",\
            "checked (Mb/s)", "std_dev","overhead(%)"]
    data_writer.writerow(header)

    normalized = []
    exp = 2  # exponent
    # Process data in each httpd bandwidth file
    for i in range(0, len(bw_baseline_file)):
        bw_baseline = bw_baseline_file[i].split(",")[:-1]
        bw_checked = bw_checked_file[i].split(",")[:-1]

        # convert string to float
        for j in range(0, len(bw_baseline)):
            bw_baseline[j] = float(bw_baseline[j]) / 1024
        for j in range(0, len(bw_checked)):
            bw_checked[j] = float(bw_checked[j]) / 1024

        # Computer arithmetic mean and standard deviation
        mean_baseline = round(np.mean(np.array(bw_baseline)), 2)
        mean_checked = round(np.mean(np.array(bw_checked)), 2)
        std_baseline = round(np.std(np.array(bw_baseline)), 2)
        std_checked = round(np.std(np.array(bw_checked)), 2)
        overhead = (mean_baseline - mean_checked) / mean_baseline * 100

        file_size = 2**exp
        if file_size < 2**10:
            data_row = [str(file_size) + " KB"]
        else:
            data_row = [str(int(file_size / (2**10))) + " MB"]
        data_row += [mean_baseline, std_baseline, mean_checked, std_checked]
        data_writer.writerow(data_row)
        exp += 1

        # check if there is an anomaly
        if (mean_checked > mean_baseline and
                (mean_checked - std_checked) > (mean_baseline + std_baseline)):
            print("The checked thttpd outperms the original one!")

        normalized += [mean_checked / mean_baseline]
        # print("mean_baseline: " + str(mean_baseline) + "; mean_checked: " + str(mean_checked))
        # print("std_baseline = " + str(std_baseline) + "; std_checked = " + str(std_checked))
        # print("overhead = " + str(overhead) + "%")
        # print()

    geomean = round((1 - (np.array(normalized).prod() ** (1.0 / len(normalized)))) * 100, 1)

    print("Checked C's normalized file transfer rate:")
    for i in range(2, 2 + len(normalized)):
        input_size = 2**i
        input_name = str(input_size)
        if input_size >= 2**10:
            input_name = str(int(input_size / 1024)) + " MB"
        else:
            input_name += " KB"
        print(input_name + ": " + str(round(normalized[i - 2], 3)))

    # Print summarized results.
    print()
    print("Performance overhead summary:")
    Min = round((1 - max(normalized)) * 100, 1)
    Max = round((1 - min(normalized)) * 100, 1)
    print("Min =  " + str(Min) + "%")
    print("Max = " + str(Max) + "%")
    print("Geomean = " + str(geomean) + "%")

    perf_file.close()

#
# Entrance of this script
#
def main():
    compute()

if __name__ == "__main__":
    main()

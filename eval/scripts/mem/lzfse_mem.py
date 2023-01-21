#!/usr/bin/env python3

#
# This script reads in lzfse memory consumption data and calculates the
# memory overhead incurred by Checked C.
#

import numpy as np
import os
import csv

#
# Project root directory and data directory for memory overhead.
#
ROOT_DIR = os.path.abspath(os.getcwd() + "/../../..")
DATA_DIR = ROOT_DIR + "/eval/mem_data/lzfse/"

#
# Data files
#
INPUTS = [
    "dickens",
    "mozilla",
    "mr",
    "nci",
    "ooffice",
    "osdb",
    "reymont",
    "samba",
    "sao",
    "webster",
    "xml",
    "x-ray",
    "enwik9"
]

#
# RSS threshold: We only process data files whose RSS are greater than a threshold
#
RSS_THRESHOLD = 10

#
# Memory consumption data
#
mem_data = {
        "baseline" : {
            "encode" : {
                "rss" : {}, "rss_max" : {},
                "wss" : {}, "wss_max" : {}
            },
            "decode" : {
                "rss" : {}, "rss_max" : {},
                "wss" : {}, "wss_max" : {}
            },
        },
        "checked" : {
            "encode" : {
                "rss" : {}, "rss_max" : {},
                "wss" : {}, "wss_max" : {}
            },
            "decode" : {
                "rss" : {}, "rss_max" : {},
                "wss" : {}, "wss_max" : {}
            },
        },
}

#
# Round a float number to its nearest integer.
#
def Int(num):
    return int(round(num, 0))

#
# Collect data in the output files from wss.pl, and compute the average and
# max RSS and average WSS.
#
# @param setting : "baseline" or "checked"
# @param task    : "encode"   or "decode"
#
def collect_data(setting, task):
    for data_name in INPUTS:
        data_file = open(DATA_DIR + setting + "/" + data_name + "_" + task + ".stat")
        # Skip the headings and the last ERROR reporting line.
        data = data_file.readlines()[2:-1]
        # Skip over empty data
        if not data:
            continue
        rss, wss = [], []
        for line in data:
            line = line.split()
            rss_one, wss_one = round(float(line[1]), 2),round(float(line[3]), 2)
            if wss_one == 0:
                break
            rss += [rss_one]
            wss += [wss_one]
        if not wss:
            # This means the first line of data has a 0 WSS.
            continue
        # Get the maximum RSS
        mem_data[setting][task]["rss_max"][data_name] = round(max(rss), 2)
        mem_data[setting][task]["wss_max"][data_name] = round(max(wss), 2)
        # Compute the average RSS and WSS.
        mem_data[setting][task]["rss"][data_name] = round(np.mean(rss), 2)
        mem_data[setting][task]["wss"][data_name] = round(np.mean(wss), 2)

#
# Write results to a CSV file.
#
def write_result():
    with open(DATA_DIR + "mem.csv", "w") as mem_csv:
        writer = csv.writer(mem_csv)
        header = ["data", "en_base_rss(MB)", "en_checked_rss(x)",\
                "en_base_wss(MB)", "en_checked_wss(x)",\
                "de_base_rss(MB)", "de_checked_rss(x)",\
                "de_base_wss(MB)", "de_checked_wss(x)"]
        writer.writerow(header)

        en_rss_norm, en_wss_norm = [], []
        de_rss_norm, de_wss_norm = [], []
        for data_name in INPUTS:
            row = [data_name]

            # Add encode rss data
            if data_name in mem_data["baseline"]["encode"]["rss_max"] and \
               data_name in mem_data["checked"]["encode"]["rss_max"]:
                   en_base_rss = mem_data["baseline"]["encode"]["rss_max"][data_name]
                   en_checked_rss = mem_data["checked"]["encode"]["rss_max"][data_name]
                   normalized = round(en_checked_rss / en_base_rss, 3)
                   en_rss_norm += [normalized]
                   # Add baseline encoding for RSS.
                   row += [Int(en_base_rss)]
                   # Add normalized Checked C encoding for RSS.
                   row += [normalized]
            else:
                row += ["", ""]

            # Add encode wss data
            if data_name in mem_data["baseline"]["encode"]["wss"] and \
               data_name in mem_data["checked"]["encode"]["wss"]:
                   en_base_wss = mem_data["baseline"]["encode"]["wss"][data_name]
                   en_checked_wss = mem_data["checked"]["encode"]["wss"][data_name]
                   normalized = round(en_checked_wss / en_base_wss, 3)
                   en_wss_norm += [normalized]
                   # Add baseline encoding for WSS.
                   row += [Int(en_base_wss)]
                   # Add normalized Checked C encoding for RSS.
                   row += [normalized]
            else:
                row += ["", ""]

            # Add decode rss data
            if data_name in mem_data["baseline"]["decode"]["rss_max"] and \
               data_name in mem_data["checked"]["decode"]["rss_max"]:
                   de_base_rss = mem_data["baseline"]["decode"]["rss_max"][data_name]
                   de_checked_rss = mem_data["checked"]["decode"]["rss_max"][data_name]
                   normalized = round(de_checked_rss / de_base_rss, 3)
                   de_rss_norm += [normalized]
                   # Add baseline decoding for RSS.
                   row += [Int(de_base_rss)]
                   # Add normalized Checked C decoding for RSS.
                   row += [normalized]
            else:
                row += ["", ""]

            # Add decode wss data
            if data_name in mem_data["baseline"]["decode"]["wss"] and \
               data_name in mem_data["checked"]["decode"]["wss"]:
                   de_base_wss = mem_data["baseline"]["decode"]["wss"][data_name]
                   de_checked_wss = mem_data["checked"]["decode"]["wss"][data_name]
                   normalized = round(de_checked_wss / de_base_wss, 3)
                   de_wss_norm += [normalized]
                   # Add baseline decoding for RSS.
                   row += [Int(de_base_wss)]
                   # Add normalized Checked C decoding for RSS.
                   row += [normalized]
            else:
                row += ["", ""]

            for i in range(1, len(row)):
                if row[i] != "":
                    writer.writerow(row)
                    break


        # Compute the geomean of Checked C's and CETS' RSS and WSS.
        data_num = len(en_rss_norm)
        en_rss_geomean = round(np.array(en_rss_norm).prod() ** (1.0 / data_num), 3)
        en_wss_geomean = round(np.array(en_wss_norm).prod() ** (1.0 / data_num), 3)
        data_num = len(de_rss_norm)
        de_rss_geomean = round(np.array(de_rss_norm).prod() ** (1.0 / data_num), 3)
        de_wss_geomean = round(np.array(de_wss_norm).prod() ** (1.0 / data_num), 3)

        row = ["geomean", "", en_rss_geomean, de_rss_geomean, "",\
                en_wss_geomean, de_wss_geomean]
        writer.writerow(row)

        # Close result file.
        mem_csv.close()

        # Print the summarized data.

        print("Checked C's RSS overhead on lzfse encoding:")
        print_max_min(en_rss_norm, "RSS Min", True)
        print_max_min(en_rss_norm, "RSS Max", False)
        print_geomean(en_rss_geomean)

        print("")
        print("Checked C's RSS overhead on lzfse decoding:")
        print_max_min(de_rss_norm, "RSS Min", True)
        print_max_min(de_rss_norm, "RSS Max", False)
        print_geomean(de_rss_geomean)

        print("")
        print("Checked C's WSS overhead on lzfse encoding:")
        print_max_min(en_wss_norm, "WSS Min", True)
        print_max_min(en_wss_norm, "WSS Max", False)
        print_geomean(en_wss_geomean)

        print("")
        print("Checked C's WSS overhead on lzfse decoding:")
        print_max_min(de_wss_norm, "WSS Min", True)
        print_max_min(de_wss_norm, "WSS Max", False)
        print_geomean(de_wss_geomean)

#
# Print out the max/min overhead.
#
# @data: An array of normalized memory consumption.
# @ msg: Type of data, such as "RSS Min"
# @is_min: Whether to print out max or min of the data
#
# @output: Memory overhead in the format of "x%".
#
def print_max_min(data, msg, is_min):
    if is_min:
        print(msg + " = " + str(round((min(data) - 1) * 100, 2)) + "%")
    else:
        print(msg + " = " + str(round((max(data) - 1) * 100, 2)) + "%")

#
# Print out the geomean in the format of "%x".
#
def print_geomean(data):
    print("Geomean = " + str(round((data - 1) * 100, 2)) + "%")


def print_mem_consumption():
    print("Baseline encoding RSS:")
    print(mem_data["baseline"]["encode"]["rss_max"])
    print("Checked C encoding RSS:")
    print(mem_data["checked"]["encode"]["rss_max"])
    print("Baseline encoding WSS:")
    print(mem_data["baseline"]["encode"]["wss"])
    print("Checked C encoding WSS:")
    print(mem_data["checked"]["encode"]["wss"])
    print("Baseline decoding RSS:")
    print(mem_data["baseline"]["decode"]["rss_max"])
    print("Checked C decoding RSS:")
    print(mem_data["checked"]["decode"]["rss_max"])
    print("Baseline decoding WSS:")
    print(mem_data["baseline"]["decode"]["wss"])
    print("Checked C decoding WSS:")
    print(mem_data["checked"]["decode"]["wss"])


#
# Entrance of this script
#
def main():
    # Collect all the raw data generated by wss.
    collect_data("baseline", "encode")
    collect_data("baseline", "decode")
    collect_data("checked", "encode")
    collect_data("checked", "decode")

    # Write results to a csv file
    write_result()

if __name__ == "__main__":
    main()

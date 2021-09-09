#!/usr/bin/env python3

'''
This script computes the performance of lzfse
It assumes that compression and decompression result files have alreayd been
generated.
'''

import csv
import os
import numpy as np

ITER = 20
ROOT_DIR = os.path.abspath(os.getcwd() + "/../..")
DATA_DIR = ROOT_DIR + "/eval/perf_data/lzfse/"
BASELINE_DATA_DIR = DATA_DIR + "baseline/"
CHECKED_DATA_DIR = DATA_DIR + "checked/"

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

FILE_SIZE = {
    "dickens"  : "9.8 MB",
    "mozilla"  : "49 MB",
    "mr"       : "9.6 MB",
    "nci"      : "32 MB",
    "ooffice"  : "5.9 MB",
    "osdb"     : "9.7 MB",
    "reymont"  : "6.4 MB",
    "samba"    : "21 MB",
    "sao"      : "7.0 MB",
    "webster"  : "40 MB",
    "xml"      : "5.1 MB",
    "x-ray"    : "8.1 MB",
    "enwik9"   : "954 MB",
}

#
# 4 setups.
#
configs =[
    {
        "rate_file_dir" : BASELINE_DATA_DIR + "compress/",
        "data_rates"    : { }
    },
    {
        "rate_file_dir" : CHECKED_DATA_DIR + "compress/",
        "data_rates"    : { }
    },
    {
        "rate_file_dir" : BASELINE_DATA_DIR + "decompress/",
        "data_rates"    : { }
    },
    {
        "rate_file_dir" : CHECKED_DATA_DIR + "decompress/",
        "data_rates"    : { }
    },
]

#
# Main body of this script
#
def perf():
    for config in configs:
        data_rates = config["data_rates"]
        for data in INPUTS:
            for i in range(1, ITER + 1):
                # Read in raw data
                rate_file = open(config["rate_file_dir"] + data + "." + str(i))
                rate = float(rate_file.readlines()[-1].split(',')[-1].split()[0])
                if data in data_rates:
                    data_rates[data] += rate
                else:
                    data_rates[data] = rate
            data_rates[data] = round(data_rates[data] / ITER, 1)

    # Write results to a csv file
    with open(DATA_DIR + "perf.csv", 'w') as perf_csv:
        writer = csv.writer(perf_csv)
        header = ["file", "size(MB)",
                  "en_baseline(MB/s)", "en_checked(MB/s)", "en_overhead(%)",
                  "de_baseline(MB/s)", "de_checked(MB/s)", "de_overhead(%)"]
        writer.writerow(header)

        en_normalized, de_normalized = [], []
        for data in INPUTS:
            row = [data, FILE_SIZE[data].split()[0]]
            en_baseline = configs[0]["data_rates"][data]
            en_checked  = configs[1]["data_rates"][data]
            de_baseline = configs[2]["data_rates"][data]
            de_checked  = configs[3]["data_rates"][data]
            en_overhead = round((en_baseline - en_checked) / en_baseline * 100, 1)
            de_overhead = round((de_baseline - de_checked) / de_baseline * 100, 1)
            en_normalized += [en_checked / en_baseline]
            de_normalized += [de_checked / de_baseline]
            row += [en_baseline, en_checked, en_overhead,
                    de_baseline, de_checked, de_overhead]
            writer.writerow(row)

    # Print summarized data for compression
    print("Min(encode_overhead) = " + str(round((1 - max(en_normalized)) * 100, 1)) + "%")
    print("Max(encode_overhead) = " + str(round((1 - min(en_normalized)) * 100, 1)) + "%")
    en_geomean = round(np.array(en_normalized).prod() ** (1.0 / len(INPUTS)), 3)
    print("Geomean(encode_overhead): " + str(round((1 - en_geomean) * 100, 1)) + "%")
    print()
    # Print summarized data for decompression
    print("Min(decode_overhead) = " + str(round((1 - max(de_normalized)) * 100, 1)) + "%")
    print("Max(decode_overhead) = " + str(round((1 - min(de_normalized)) * 100, 1)) + "%")
    de_geomean = round(np.array(de_normalized).prod() ** (1.0 / len(INPUTS)), 2)
    print("Geomean(decode_overhead): " + str(round((1- de_geomean) * 100 ,1)) + "%")

#
# Entrance of this script
#
def main():
    perf()

if __name__ == "__main__":
    main()


#!/usr/bin/env python3

#
# This script merges the Checked C and CETS performance result files into
# on csv file ready for gnuplot to use.
#

import csv
import sys

DATA_DIR = "../perf_data/olden/"

SKIPPED = ["bh", "em3d", "mst"]

#
# Main body of this script
#
def merge():
    checked_data = DATA_DIR + "checked.csv"
    cets_data = DATA_DIR + "cets.csv"

    checked = open(checked_data, 'r').readlines()
    cets = open(cets_data, 'r').readlines()

    if len(checked) != len(cets):
        sys.exit("Checked C and CETS result data files have different lines")

    # Collect data
    merged = []
    for i in range(1, len(checked)):
        benchmark = checked[i].split(',')[0]
        normalized_checked = checked[i].split(',')[3].rstrip()
        normalized_cets = ''
        if benchmark not in SKIPPED:
            normalized_cets = cets[i].split(',')[3].rstrip()
        merged += [(benchmark, (normalized_checked, normalized_cets))]

    # Write to a csv file
    with open(DATA_DIR + "checked_cets.csv", 'w') as result:
        writer = csv.writer(result)
        header = ["benchmark", "normalized(x)"]
        writer.writerow(header)

        for data in merged:
            row = [data[0], data[1][0], data[1][1]]
            writer.writerow(row)


def main():
    merge()

if __name__ == "__main__":
    main()


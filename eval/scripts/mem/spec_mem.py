#!/usr/bin/env python3

'''
This script computes the memory overhead of 429.mcf.
'''

import numpy as np
import os
import csv

ROOT_DIR = os.path.abspath(os.getcwd() + "/../../..")
DATA_DIR = ROOT_DIR + "/eval/mem_data/spec/"

PROGRAM = "429.mcf"

#
# Collect memory data.
#
def collect_data(setting):
    # Skip the firt 2% of one run (warmup period)
    skip_warmup = 4
    if setting == "checked":
        skip_warmup = 6

    lines = open(DATA_DIR + setting + "/" + PROGRAM).readlines()[2 + skip_warmup: -1]
    rss, wss = [], []
    for line in lines:
        data = line.split()
        rss += [float(data[1])]
        wss += [float(data[3])]

    rss_max = max(rss)
    wss_mean = round(sum(wss) / len(wss), 2)

    return rss_max, wss_mean

#
# Entrance of this program.
#
if __name__ == "__main__":
    baseline_rss, baseline_wss = collect_data("baseline")
    checked_rss, checked_wss = collect_data("checked")

    print("Baseline RSS: " + str(baseline_rss))
    print("Checked RSS: " + str(checked_rss))
    print("Overhead: " + str(round((checked_rss / baseline_rss - 1) * 100, 2)) + "%")
    print()

    print("Baseline WSS: " + str(baseline_wss))
    print("Checked WSS: " + str(checked_wss))
    print("Overhead: " + str(round((checked_wss / baseline_wss - 1) * 100, 2)) + "%")

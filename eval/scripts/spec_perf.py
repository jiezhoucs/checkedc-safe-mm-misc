#!/usr/bin/env python3

'''
This script collects and computes Checked C's performance overhead on 429.mcf.
'''

import numpy as np
import os
import subprocess as sp

ROOT_DIR = os.path.abspath(os.getcwd() + "/../../")
DATA_DIR = ROOT_DIR + "/eval/perf_data/spec/"

ITER = 20

#
# Collect exec_time from the result json files.
#
def collect_data(setting):
    # For some mysterious reason, the next line does not wokr (stuck) while
    # the similar one in count_mm.py works.
    # baseline_lines = sp.run(["grep", "exec_time", "*.json"], stdout=sp.PIPE).stdout.decode("utf-8").split('\n')[:-1]

    total_time = 0
    os.chdir(DATA_DIR + setting)
    for f in os.listdir():
        exec_time = sp.run(["grep", "exec_time", f], stdout=sp.PIPE).\
                stdout.decode("utf-8").strip().split(': ')[1][:-1]
        total_time += float(exec_time)

    return round(total_time / ITER, 2)

#
# Entrance of this script
#
if __name__ == "__main__":
    baseline_time = collect_data("baseline")
    checked_time = collect_data("checked")

    print("Baseline: " + str(baseline_time))
    print("Checked: " + str(checked_time))
    print("Overhead: " + str(round((checked_time / baseline_time - 1) * 100, 2)) + "%")

#!/usr/bin/env python3

'''
This library serves python scripts that collect and compute experimental results.
'''

import numpy as np
import csv
import subprocess
from pathlib import Path

EVAL_DIR = Path(__file__).resolve().parent / ".."
SCRIPTS_DIR = EVAL_DIR / "scripts"
DATA_DIR_ROOT = EVAL_DIR / "perf_data"

CHECKEC = "Checked C"

def get_iter_number(benchmark):
    ''' Get the iteration number from the script that runs a benchmark (suite) '''
    cmd = f"grep 'ITER=' {benchmark} | cut -d '=' -f2"
    return int(subprocess.run(cmd, stdout=subprocess.PIPE, shell=True).stdout)

def compute_geomean(data):
    ''' Compute the geomean of an array of normalized exectuin time '''
    return round(np.array(data).prod() ** (1.0 / len(data)), 3)

def compute_aligned_len(arr):
    ''' Compute the max length of an array of strings '''
    return max(len(elem) for elem in arr)

def convert_normalized_to_overhead(val, time=True, decimal_precision=1):
    '''
    Convert a normalized execution time to a percentage, e.g., 1.21 -> 21.0%.
    The second parameter @time indicates whether the first parameter @val
    represents execution time. When false, @val represents a metric where a
    higher value indciates better performance, such as compression rate.
    '''
    overhead = val - 1 if time == True else 1 - val
    return f"{round(overhead * 100, decimal_precision)}%"

def print_summarized_overhead(min_norm, max_norm, geomean, compiler, time=True):
    ''' Print min, max, and geomean of overhead'''
    if not time :
        min_overhead, max_overhead = max_norm, min_norm
    else:
        min_overhead, max_overhead  = min_norm, max_norm

    print(f"{compiler}'s summarized overhead:")
    print(f"Min     = {convert_normalized_to_overhead(min_overhead, time)}")
    print(f"Max     = {convert_normalized_to_overhead(max_overhead, time)}")
    print(f"Geomean = {convert_normalized_to_overhead(geomean, time)}")
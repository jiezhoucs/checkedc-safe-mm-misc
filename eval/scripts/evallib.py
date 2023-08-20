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
DATA_ROOT_DIR = EVAL_DIR / "perf_data"

def get_iter_number(benchmark):
    ''' Get the iteration number from the script that runs a benchmark (suite) '''
    cmd = f"grep 'ITER=' {benchmark} | cut -d '=' -f2"
    return int(subprocess.run(cmd, stdout=subprocess.PIPE, shell=True).stdout)

def compute_geomean(data):
    ''' Compute the geomean of an array of normalized exectuin time '''
    return round(np.array(data).prod() ** (1.0 / len(data)), 3)

def convert_normalized_to_percent(val):
    ''' Convert a normalized execution time to a percentage, e.g., 1.21 -> 21.0%'''
    return f"{round((val - 1) * 100, 1)}%"

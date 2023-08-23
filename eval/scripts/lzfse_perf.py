#!/usr/bin/env python3

'''
This script computes the performance of lzfse using outputs of (de)compression rates.
'''

from evallib import *
import os

LZFSE_RUN_SH = SCRIPTS_DIR / "lzfse_run.sh"
DATA_DIR = DATA_DIR_ROOT / "lzfse"
DATA_DIR_BASELINE = DATA_DIR / "baseline"
DATA_DIR_CHECKED = DATA_DIR / "checked"

INPUTS = {
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

def get_rate_from_file(filepath):
    ''' Read (de)compression rate from a result file '''
    with open(filepath) as rate_file:
        return float(rate_file.readlines()[-1].split(',')[-1].split()[0])

def compute_perf(iter):
    ''' Main body of this script '''

    configs = [
        {"rate_file_dir" : dir, "data_rates" : {}}
        for dir in [DATA_DIR_BASELINE / "compress", DATA_DIR_CHECKED / "compress",
                    DATA_DIR_BASELINE / "decompress", DATA_DIR_CHECKED / "decompress"]
    ]

    # Read rates from output files.
    for config in configs:
        data_rates = config["data_rates"] # A convenient alias
        for data in INPUTS:
            total_rates = sum(get_rate_from_file(f"{config['rate_file_dir']}/{data}.{i}")\
                              for i in range(1, iter + 1))
            data_rates[data] = round(total_rates / iter, 1)

    # Write results to a csv file
    with open(DATA_DIR / "perf.csv", 'w') as perf_csv:
        writer = csv.writer(perf_csv)
        header = ["file", "size(MB)",
                  "en_base(MB/s)", "en_check(MB/s)", "en_norm(%)",
                  "de_base(MB/s)", "de_check(MB/s)", "de_norm(%)"]
        writer.writerow(header)

        en_normalized, de_normalized = {}, {}
        for data_name in INPUTS:
            row = [data_name, INPUTS[data_name].split()[0]]
            en_baseline = configs[0]["data_rates"][data_name]
            en_checked  = configs[1]["data_rates"][data_name]
            de_baseline = configs[2]["data_rates"][data_name]
            de_checked  = configs[3]["data_rates"][data_name]
            en_normalized[data_name] = en_checked / en_baseline
            de_normalized[data_name] = de_checked / de_baseline
            row += [en_baseline, en_checked, round(en_normalized[data_name], 2),
                    de_baseline, de_checked, round(de_normalized[data_name], 2)]
            writer.writerow(row)

        en_geomean = compute_geomean([rate for rate in en_normalized.values()])
        de_geomean = compute_geomean([rate for rate in de_normalized.values()])

        row = ["Geomean", "", "", "", en_geomean, "", "", de_geomean]
        writer.writerow(row)

    # Print results for compression
    print_normalized(en_normalized, "compression")
    min_norm, max_norm = min(en_normalized.values()), max(en_normalized.values())
    print_summarized_overhead(min_norm, max_norm, en_geomean, CHECKEC, False)

    print()

    # Print results for decompression
    print_normalized(de_normalized, "decompression")
    min_norm, max_norm = min(de_normalized.values()), max(de_normalized.values())
    print_summarized_overhead(min_norm, max_norm, de_geomean, "Checked C", False)

def print_normalized(data, mode):
    ''' Print normalized perf data. @mod is "compression" or "decompression '''

    print(f"Checked C's normalized execution time of {mode}:")
    for data_name in INPUTS:
        print(f"{data_name:<{compute_aligned_len(INPUTS.keys())}} : {round(data[data_name], 2)}")

    print()

#
# Entrance of this script
#
if __name__ == "__main__":
    compute_perf(get_iter_number(LZFSE_RUN_SH))
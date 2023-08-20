#!/usr/bin/env python3

'''
This script collects Olden performance data, computes performance overhead
of Checked C and CETS, and writes the results to three files
    - checked.csv
    - cets.csv
    - perf.csv (containing both checked and cets data)
'''

from evallib import *
import json

DATA_DIR = DATA_ROOT_DIR / "olden"
OLDEN_RUN_SH = SCRIPTS_DIR / "olden_run.sh"

BENCHMARKS = [
    "bh",
    "bisort",
    "em3d",
    "health",
    "mst",
    "perimeter",
    "power",
    "treeadd",
    "tsp",
]

CETS_SKIPPED = ["bh", "em3d", "mst"]

exec_time_baseline, exec_time_checked, exec_time_cets = {}, {}, {}
normalized_checked, normalized_cets = {}, {}

def load_output_json(path):
    '''A helper function to load a json data file'''
    with open(path, 'r') as f:
        return json.load(f)

def collect_data(iter):
    ''' Collect performance data '''
    global exec_time_baseline, exec_time_checked, exec_time_cets
    global normalized_checked, normalized_cets

    # Initialize the execution time to 0
    for prog in BENCHMARKS:
        exec_time_baseline[prog] = exec_time_checked[prog] = 0
        if prog not in CETS_SKIPPED:
            exec_time_cets[prog] = 0
    
    # A heler lambda to compute the path of the output JSON file.
    output_json = lambda target, prog, i: DATA_DIR / target / f"{prog}.{i}.json"

    # Use Python's JSON module to extract the execution time.
    for prog in BENCHMARKS:
        for i in range(1, iter + 1):
            data = load_output_json(output_json("baseline", prog, i))
            exec_time_baseline[prog] += float(data['tests'][0]['metrics']['exec_time'])

            data = load_output_json(output_json("checked", prog, i))
            exec_time_checked[prog] += float(data['tests'][0]['metrics']['exec_time'])

            if prog not in CETS_SKIPPED:
                data = load_output_json(output_json("cets", prog, i))
                exec_time_cets[prog] += float(data['tests'][0]['metrics']['exec_time'])

    # Calculate arithemetic mean of execution time and normalize the result
    for prog in BENCHMARKS:
        exec_time_baseline[prog] /= iter
        exec_time_checked[prog] /= iter
        normalized_checked[prog] = exec_time_checked[prog] / exec_time_baseline[prog]

        if prog not in CETS_SKIPPED:
            exec_time_cets[prog] /= iter
            normalized_cets[prog] = exec_time_cets[prog] / exec_time_baseline[prog]

def print_normalized(data, compiler):
    ''' Print normalized execution time '''
    benchmarks = (
        BENCHMARKS if compiler == "Checked C"
        else [prog for prog in BENCHMARKS if prog not in CETS_SKIPPED]
    ) 

    aligned_length = max(len(prog) for prog in data)

    print(f"{compiler}'s normalized execution time:")
    for prog in benchmarks:
        print(f"{prog:<{aligned_length}} : {round(data[prog], 2)}")
    print("")

def print_summarized(min_time, max_time, geomean, compiler):
    ''' Print summarized performance overhead '''
    print(f"{compiler}'s summarized performance overhead:")
    print(f"Min     = {convert_normalized_to_percent(min_time)}")
    print(f"Max     = {convert_normalized_to_percent(max_time)}")
    print(f"Geomean = {convert_normalized_to_percent(geomean)}")

def write_result():
    '''
    Compute geomean of perf overhead and  write the result to three files.
    '''

    # Write Checked C's result to a CSV file
    with open(DATA_DIR / "checked.csv", "w") as checked_csv:
        writer = csv.writer(checked_csv)
        header = ["program", "baseline(s)", "checked(s)", "normalized(x)", "overhead(%)"]
        writer.writerow(header)

        # Write normalized execution time of each benchmark
        for prog in BENCHMARKS:
            row = [prog]
            row += [round(exec_time_baseline[prog], 2)]
            row += [round(exec_time_checked[prog], 2)]
            row += [round(normalized_checked[prog], 3)]
            row += [round((row[-1] - 1) * 100, 1)]
            writer.writerow(row)

        # Compute and write geomean
        overhead_checked, overhead_checked_cets = [], []
        for prog in BENCHMARKS:
            overhead_checked += [normalized_checked[prog]]
            if prog not in CETS_SKIPPED:
                overhead_checked_cets += [overhead_checked[-1]]
        geomean_checked = compute_geomean(overhead_checked)
        geomean_checked_cets = compute_geomean(overhead_checked_cets)
        min_checked, max_checked = min(overhead_checked), max(overhead_checked)
        row = ["Geomean", '', '', geomean_checked, round((geomean_checked - 1) * 100, 1)]
        writer.writerow(row)

    # Write CETS's results to a CSV file
    with open(DATA_DIR / "cets.csv", "w") as cets_csv:
        writer = csv.writer(cets_csv)
        header = ["program", "baseline(s)", "cets(s)", "normalized(x)", "overhead(%)"]
        writer.writerow(header)

        for prog in BENCHMARKS:
            row = [prog]
            if prog in CETS_SKIPPED:
                row += ['', '', '', '']
            else:
                row += [round(exec_time_baseline[prog], 2)]
                row += [round(exec_time_cets[prog], 2)]
                row += [round(normalized_cets[prog], 3)]
                row += [round((row[-1] - 1) * 100, 1)]
            writer.writerow(row)

        # Compute and write geomean
        overhead_cets = []
        for prog in BENCHMARKS:
            if prog not in CETS_SKIPPED:
                overhead_cets += [normalized_cets[prog]]
        geomean_cets = compute_geomean(overhead_cets)
        min_cets, max_cets = min(overhead_cets), max(overhead_cets)
        row = ["Geomean", '', '', geomean_cets, round((geomean_cets - 1) * 100, 1)]
        writer.writerow(row)

    # Write the summarized Checked C vs. CETS data to a csv file
    with open(DATA_DIR / "perf.csv", "w") as perf_csv:
        writer = csv.writer(perf_csv)
        header = ["program", "baseline(s)", "checked(s)", "normalized_checked(x)",\
                  "cets(s)", "normalized_cets(x)"]
        writer.writerow(header)

        for prog in BENCHMARKS:
            row = [prog]
            row += [round(exec_time_baseline[prog], 2)]
            row += [round(exec_time_checked[prog], 2)]
            row += [round(normalized_checked[prog], 3)]
            if prog not in CETS_SKIPPED:
                row += [round(exec_time_cets[prog], 2)]
                row += [round(normalized_cets[prog], 3)]
            else:
                row += ["", ""]
            writer.writerow(row)
        row = ['Geomean', '', '', geomean_checked, '', geomean_cets]
        writer.writerow(row)

    # Print detailed data
    print_normalized(normalized_checked, "Checked C")
    print_summarized(min_checked, max_checked, geomean_checked, "Checked C")
    print(f"CETS-only Geomean = {convert_normalized_to_percent(geomean_checked_cets)}")

    print("")

    print_normalized(normalized_cets, "CETS")
    print_summarized(min_cets, max_cets, geomean_cets, "CETS")

#
#  Entrance of this script
#
if __name__ == "__main__":
    collect_data(get_iter_number(OLDEN_RUN_SH))

    write_result()
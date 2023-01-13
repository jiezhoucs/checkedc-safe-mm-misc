# Artifact Evaluation Instructions for the OOPSLA'23 Checked C Paper

## Overview
This artifact is a virtual machine of Ubuntu 20.04 that contains

- the three compilers (baseline LLVM, our Checked C, and CETS)
- all the scripts that run benchmarks, collects data, and computes results

All materials used in this AE are in directory `/home/ae1/ae`.

### Important Scripts
There are three scripts that the users will need to run:

```shell
- dryrun_mini.sh  # Quick test on compilers and benchmarks.
- eval.sh         # Main script for evaluation.
- update.sh       # An auxiliary script to `git pull` changes in the `misc` repo.
```

### Password of the account ae1
`checkedc`

### Important Repositories

- [Checked C clang](https://github.com/jzhou76/checkedc-clang)
- [Checked C llvm](https://github.com/jzhou76/checkedc-llvm)
- [misc](https://github.com/jzhou76/checkedc-safe-mm-misc)
- [llvm test-suite](https://github.com/jzhou76/test-suite)

Our Checked C compiler repositories were forked from Microsoft's Checked C
compiler repos. They have the complete development history of our extension
to the original Checked C compiler. In addition, the [misc](https://github.com/jzhou76/checkedc-safe-mm-misc)
repo and the [test-suite](https://github.com/jzhou76/test-suite) contain the
porting history of the benchmarks (except `429.mcf` which belongs to SPEC 2006)
used in the paper.

### Project Organization
The whole project is organized as the README of the `checkedc-safe-mm-misc`
repository shows https://github.com/jzhou76/checkedc-safe-mm-misc#directory-organization

## Evaluation

### Benchmarks (Table 3 of the paper)

- Olden (located in `llvm-test-suite`)
- parson
- lzfse
- thttpd
- curl

We did not include `429.mcf` as it belongs to the SPEC CPU2006, which is a
proprietary benchmark suite.


### Min Dryrun

We provide a script `dryrun_mini.sh` in the root directory `ae` to do a quick
test on the compilers and benchmarks. Specifically,

- For `Olden`, it only tests `bisort` to save time.
- For `parson`, it tests parson's built-in tests.
- For `lzfse`, it compresses and decompresses one of the input files.
- For `thttpd`, it transfers one 16~KB test file.
- For `curl`, it runs 10 of curl's built-in test cases.

The script will clean existing binaries and compile for `Olden`, `parson`,
and `lzfse` on the spot. It does not compile `thttpd` and `curl` because the
output is very verbose, which would hinder the readability of the output of the
execution of the benchmarks.

Without any command line argument, this script will compile and run all
benchmarks.  However, we recommend users to provide the name of a benchmark
as the first argument, e.g.,

```shell
dryrun_mini.sh olden
```

will test a single benchmark and it is easier to examine the output.
The benchmarks have clear output to show weather they finish successfully.

### Evaluation

Running the `eval.sh` script without any arguments will run all benchmarks.
Similar to `dryrun_mini.sh`, one can choose to run a specific benchmark, e.g.,

```shell
eval.sh olden
```

will run all `olden` benchmarks and generate performance and memory consumption
data. It will also compute the final results based on the raw data.

### Collecting Data

Both the raw data and the summarized data will be put in `misc/eval/perf_data`
and `misc/eval/mem_data`. `misc/eval/perf_data/benchmark/perf.csv` contains
the data that were used to draw the figures in Fig. 5 of the paper. Similarly,
those `misc/eval/mem_data/benchmark/mem.csv` were used for Table 4 of the paper.
To show the experimental data separately, run

```shell
./print_results.sh benchmark  # "olden", parson", "lzfse", "thttpd", or "curl"
```

Specifically,

```shell
./print_results.sh olden   # For Fig. 5(a) and Table 4

./print_results parson     # For Fig. 5(c) and Table 4

./print_results lzfse      # For Fig. 5(d) and Table 4

./print_results curl       # For Section 6.3.5 and Table 4

./print_result thttpd      # For Fig. 5(b) and Table 4
```

#### Data Inconsistencies

Due to the short execution time, there may be noticeable differences between
what users get on their machine and Fig. 5(c) and Table 4 for `parson`.

In addition, we use fixed time interval (details in the scripts in
`misc/eval/scripts/mem`) to measure memory overhead, and therefore in a
different environment, users may observe different memory consumption largely
due to the different execution time of a program.

# Artifact Evaluation Instructions for the OOPSLA'23 Checked C Paper

First, we would like to apologize for not providing a VM or a Docker container
for this AE. We have no such prior experience and we were on a very tight
schedule so we do not have the time to prepare one. We therefore do not provide
pre-installed software packages but instead giving scripts to download and
build our artifacts on your machine. We understand that this may cause extra
trouble in the process, such as requiring users to install dependency software.
However, we also argue that this approach has an advantage over a docker-based
approach: it would be easier to show the robustness and reusability of
this artifact.

## Overview
This artifact has the following scripts:

```shell
- setup.sh            # Set up the experimental environment.
- eval.sh             # Compile and run the benchmarks.
- print_results.sh    # Print out evaluation results.
- thttpd.sh           # Build, install, and run thttpd. See later for details.
```

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

## Software Dependencies
The scripts mentioned in the **Overview** section requires the following
dependencies:

```shell
- cmake       # For building llvm, llvm test-suite, and lzfse
- git         # For downloading repos
- git-lfs     # For pulling down large input data files for evaluation
- wget        # For downloading the baseline llvm compiler
- unzip       # For unpackaging enwik9.
- python2     # llvm-lit uses `#/usr/bin/env python`, which could be python2
- python3     # For processing experimental data
- pip3        # For checking (and installing) numpy
- ab          # For evaluating thttpd. Available in "apache2-utils"
- autoconf    # For generating Makefile for curl
```

In addition, we assumed that the host system has a C/C++ compiler. We recommend
using `clang/clang++` to compile all the three compilers of this artifact.
We tested using `clang-10`, `clang-11`, and `clang-13` on three systems
respectively to compile the compilers of this artifact.
`gcc/g++` may raise unexpected compilation errors.

We recommend running the scripts on a normal working Linux machine to save the
trouble of installing most of the dependencies and avoiding having unexpected
errors such as missing libraries. Most of the dependencies are commonly used
software except `ab`. `ab` is the Apache Benchmarking tool for benchmarking
HTTP servers. On a Debian system, it can be install via the `apache2-utils`
package.

## Setting Up the Environment

Please put the scripts into a new directory, which would serve as the root
directory for the whole project. The name of the root directory does not matter.

To set up the environment:

```shell
./setup.sh
```

It downloads and builds the three compilers (baseline, Checked C, and CETS).
It also configures the llvm test-suite for evaluating the compilers on the Olden
benchmark suite. In addition, it prepares large data input that is not suitable
for storing directly on Github (Github charges for large files).

## Evaluation

### Benchmarks (Table 3 of the paper)

- Olden (located in `llvm-test-suite`)
- parson
- lzfse
- thttpd
- curl

We did not include `429.mcf` as it belongs to the SPEC CPU2006, which is a
proprietary benchmark suite.

#### thttpd
We handle `thttpd` specially because it requires the root privilege to install
it. We understand that it could be extremely uncomfortable to run a stranger's
script with root privilege, and we sincerely apologize for this inconvenience.
To minimize the concern of the users, we have a separate script for `thttpd`.
It is small so it will be easier for users to manually inspect it. Only the
installation step requires the root privilege:

```shell
sudo ./thttpd.sh install
```

What it does internally is to `cd` to the source directory of `thttpd` and
to execute `make install` to install `thttpd`.

The build process (`./thttpd.sh build`) and evaluation (`./thttpd.sh eval`)
does not need the root privilege.

### Evaluation

Running the `eval.sh` script without any arguments will run all benchmarks.
One can choose to run a specific benchmark, e.g.,

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
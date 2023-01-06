# Artifact Evaluation Instructions for the OOPSLA'23 Checked C Paper

First, we would like to apologize for not providing a VM or a Docker container
for this AE. We have no such prior experience and we were on a very tight
schedule so we do not have the time to prepare one. We therefore do not provide
pre-installed software packages but instead giving scripts to download and
build our artifacts on your machine. We understand that this may cause extra
trouble in the process, such as requiring you to install dependency software.
However, we also argue that this approach has an advantage over a docker-based
approach: it would be easier to show the robustness and reusability of
our software.

## Overview
This artifact has the following scripts:

```shell
- setup.sh   # Set up the experimental environment.
- eval.sh    # Compile and run the benchmarks.
```

### Important Repositories

- [Checked C clang](https://github.com/jzhou76/checkedc-clang)
- [Checked C llvm](https://github.com/jzhou76/checkedc-llvm)
- [misc](https://github.com/jzhou76/checkedc-safe-mm-misc)
- [llvm test-suite](https://github.com/jzhou76/test-suite)

Our Checked C compiler repositories were forked from Microsoft's Checked C
compiler repos. They record our complete development history of our extension
to the original Checked C compiler. In addition, the [misc](https://github.com/jzhou76/checkedc-safe-mm-misc)
repo and the [test-suite](https://github.com/jzhou76/test-suite) record the
porting history of the benchmarks (except `429.mcf` which belongs to SPEC 2006)
used in the paper.

### Project Organization
The whole project is organized as the README of the `checkedc-safe-mm-misc`
repository shows https://github.com/jzhou76/checkedc-safe-mm-misc#directory-organization

## Software Dependencies
The scripts mentioned in the **Overview** section requires the following
dependencies:

```shell
-  cmake       # For building llvm, llvm test-suite, and lzfse
-  git         # For downloading repos
-  git-lfs     # For pulling down large input data files for evaluation
-  wget        # For downloading the baseline llvm compiler
-  unzip       # For unpackaging enwik9.
-  python2     # llvm-lit uses `#/usr/bin/env python`, which could be python2
-  python3     # For processing experimental data
-  pip3        # For checking (and installing) numpy
-  ab          # For evaluating thttpd. Available in "apache2-utils"
```

In addition, we assumed that the host system has a C/C++ compiler. We recommend
using `clang/clang++` to compile all the three compilers of this artifact.
We tested using `clang-10`, `clang-11`, and `clang-13` on three systems
respectively to compile the compilers of this artifact.
`gcc/g++` may raise unexpected compilation errors.

## Setting Up the Environment

```shell
./setup.sh
```

It downloads and builds the three compilers (baseline, Checked C, and CETS).
It also configures the llvm test-suite for evaluating the compilers on the Olden
benchmark suite. In addition, it prepares large data input that is not suitable
for storing directly on Github (Github charges for extra large files).

## Evaluation

### Benchmarks (Table 3 of the paper)

- Olden (located in `llvm-test-suite`)
- parson
- lzfse

We did not include `429.mcf` as it belongs to the SPEC CPU2006, which is a
proprietary benchmark suite.

### Running

Running the `eval.sh` script without any arguments will run all benchmarks.
One can choose to run a specific benchmark, e.g.,

```shell
eval.sh olden
```

will run all `olden` benchmarks and generate performance and memory consumption
data. It will also compute the final results based on the raw data. Both the
raw data and the summarized data will be put in `misc/eval/perf_data` and
`misc/eval/mem_data`.

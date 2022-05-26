# Miscellaneous stuffs about the Checked C Temporal Memory Safety Project

This repository contains miscellaneous stuffs about the temporal memory safety
for Checked C project.

## Experimental Setup

### Directory Organization
The directories of the whole project is organized in the following hierarchy:

```shell
checkedc                    # Root directory for the whole project
|-- build                   # Build directory of the LLVM compiler
|-- llvm                    # LLVM src (including clang)
|-- misc                    # This safe-mm-misc repo
|   |-- benchmarks          # Baseline and checked src code of application benchmarks
|   |-- eval                # For performance and memory overhead evaluation
|       |-- json_dataset    # JSON data set for parson
|       |-- lzfse_dataset   # The Silesia corpus and enwik9 data for lzfse
|       |-- mem_data        # Memory consumption data
|       |-- perf_data       # Performance data
|       |-- scripts         # Scripts for running perf/mem experiements and collect resulsts
|       |-- wss             # The wss submodule (https://github.com/brendangregg/wss)
|   |-- include             # Runtime library header files
|   |-- lib                 # Runtime library
|   |-- prog_data           # Empirical data about programs in Table 1 of the paper
|   |-- scripts             # Misc scripts
|       |-- analysis        # Scripts for analyzing programs for prog_data
|       |-- cets            # Scripts for configuring and running LLVM test-suite for CETS
|       |-- other scripts   # Configure, build, run compiler/benchmarks (and etc.)
|-- tests
|   |-- test-suite          # LLVM test-suite for Checked C (with checked Olden)
|   |-- ts-build            # Build directory of the modified LLVM test-suite.
|   |-- test-suite-origin   # Original LLVM test-suite code
|   |-- ts-build-origin     # Build directory of the original LLVM test-suite
|-- benchmark-build         # build directories for thttpd and SPEC
```

### Build the Compiler

We have a `cmake` script `scripts/cmake-llvm.sh` to generate Makefiles for
building the project. If you're working on a \*nix system running on a
X86 processor, just copy the script to a build directory and run it.

Suggested building procedure:
```
cd checkedc-project-root-directory

git clone https://github.com/jzhou76/checkedc-safe-mm-misc.git misc

git clone https://github.com/jzhou76/checkedc-llvm llvm

cd llvm/tools

git clone https://github.com/jzhou76/checkedc-clang.git clang

cd ../projects/checkedc-wrapper

git clone https://github.com/jzhou76/checkedc.git

cd ../../.. ; mkdir build ; cd build

cp ../misc/scripts/cmake-llvm.sh ./

./cmake-llvm.sh

make clang -jn
```

Here is Microsoft instructions on building the compiler: [Setting up your
machine and building
clang](https://github.com/microsoft/checkedc-clang/blob/master/clang/docs/checkedc/Setup-and-Build.md)

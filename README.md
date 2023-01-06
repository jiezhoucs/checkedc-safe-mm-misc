# Miscellaneous stuff of Temporal Memory Safety for Checked C Project

Note that in this document, "Checked C" refers to our extended Checked C with
temporal memory safety, instead of the original spatially-memory-safe Checked C.

## Directory Organization
The directories of the whole project is organized in the following hierarchy:

```shell
checkedc                    # Root directory for the whole project
|-- build                   # Build directory of the Checked C compiler
|-- llvm                    # src of the Checked C compiler
|-- misc                    # This safe-mm-misc repo
|   |-- ae-oopsla23         # The scripts for OOPSLA'23 artifact evaluation
|   |-- benchmarks          # Baseline and checked src of application benchmarks
|   |-- eval                # For performance and memory overhead evaluation
|       |-- json_dataset    # JSON data set for parson
|       |-- lzfse_dataset   # The Silesia corpus for lzfse
|       |-- mem_data        # Memory consumption overhead data
|       |-- perf_data       # Performance overhead data
|       |-- scripts         # Scripts for running perf/mem experiements and collecting resulsts
|       |-- wss             # For measuring memory usage (https://github.com/brendangregg/wss)
|   |-- include             # Checed C runtime library header files
|   |-- lib                 # Checked C runtime library and debugging facilities
|   |-- prog_data           # Empirical data about programs in Table 1 of the paper
|   |-- scripts             # Misc scripts for environment setup and eval
|       |-- analysis        # Scripts for analyzing programs for prog_data
|       |-- cets            # Scripts for building and running the CETS compiler
|       |-- other scripts   # Configure, build, run compiler/benchmarks (and etc.)
|-- llvm-test-suite         # LLVM test-suite (to be created by the user)
|   |-- test-suite          # LLVM test-suite for Checked C (with checked Olden)
|   |-- ts-build            # Build directory of the modified LLVM test-suite.
|   |-- test-suite-baseline # Original LLVM test-suite code
|   |-- ts-build-origin     # test-suite build directory for the baseline llvm
|   |-- ts-build-cets       # test-suite build directory for CETS
|-- benchmark-build         # build directories for thttpd and SPEC
```

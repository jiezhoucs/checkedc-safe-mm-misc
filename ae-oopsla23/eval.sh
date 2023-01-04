#!/usr/bin/bash

ROOT_DIR=`realpath .`
SCRIPTS_DIR="$ROOT_DIR/misc/scripts"
EVAL_SCRIPTS_DIR="$ROOT_DIR/misc/eval/scripts"
CETS_DIR="$ROOT_DIR/cets"

TEST_SUITE_REPO="https://github.com/jzhou76/test-suite.git"

init() {
    # Compile libsafemm
    cd "$ROOT_DIR/misc/lib"
    if [[ ! -f "libsafemm.a" ]]; then
        make
        # Compile libsafemm_lto
        make lto
    fi

    mkdir -p llvm-test-suite; cd llvm-test-suite
    if [[ ! -d test-suite ]]; then
        git clone "$TEST_SUITE_REPO"
        cd "$SCRIPTS_DIR"
        ./cmake-ts.sh lto
        cd -
    fi

    if [[ ! -d test-suite-baseline ]]; then
        # Make a copy and checkout the baseline test-suite code.
        cp -r test-suite test-suite-baseline
        cd test-suite-baseline
        git checkout baseline

        # Generate the test-suite build files for CETS
        cd "$SCRIPTS_DIR"
        ./cmake-ts-baseline.sh lto
        cd "$ROOT_DIR/llvm-test-suite"

        # Generate the test-suite build files for CETS
        cd "$SCRIPTS_DIR/cets"
        ./cmake-ts.sh lto
        cd "$ROOT_DIR"
    fi
}

#
# Olden performance evaluation.
#
olden_perf() {
    cd "$EVAL_SCRIPTS_DIR"

    echo "Running baseline Olden benchmarks"
    ./olden_run.sh baseline

    echo "Running Checked C Olden benchmarks"
    ./olden_run.sh checked

    echo "Running CETS Olden benchmarks"
    ./olden_run.sh cets

    # Compute the performance overhead of Checked C and CETS
    ./olden_perf.py
}

#
# Olden memory evaluation.
#
olden_mem() {
    cd "$EVAL_SCRIPTS_DIR/mem"
    echo "Evaluating memory overhead of Checked C and CETS on Olden"

    echo "Running baseline Olden benchmarks"
    ./olden_run.sh baseline

    echo "Running Checked C Olden benchmarks"
    ./olden_run.sh checked

    echo "Running CETS Olden benchmarks"
    ./olden_run.sh cets

    # Compute the memory overhead of Checked C and CETS
    ./olden_mem.py
}


#
# Entrance of this script
#
init

if [[ $# != 1 ]]; then
    echo "Unknown argument."\
         "Please choose \"olden\", \"thttpd\", \"parson\", or \"lzfse\"."
else
    case $1 in
        "olden")
            olden_perf
            olden_mem
            ;;
        "thttpd")
            thttpd_perf
            thttpd_mem
            ;;
        "parson")
            parson_perf
            parson_mem
            ;;
        "lzfse")
            lzfse_perf
            lzfse_mem
            ;;
        *)
            echo "Unknown benchmark names."\
                "Please choose \"olden\", \"thttpd\", \"parson\", or \"lzfse\"."
            ;;
    esac
fi



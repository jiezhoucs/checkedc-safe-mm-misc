#!/usr/bin/env bash

ROOT_DIR=`realpath .`
SCRIPTS_DIR="$ROOT_DIR/misc/scripts"
EVAL_SCRIPTS_DIR="$ROOT_DIR/misc/eval/scripts"
CETS_DIR="$ROOT_DIR/cets"

TEST_SUITE_REPO="https://github.com/jzhou76/test-suite.git"
ENWIK9_URL="http://mattmahoney.net/dc/enwik9.zip"

init() {
    # Compile libsafemm
    cd "$ROOT_DIR/misc/lib"
    if [[ ! -f "libsafemm.a" ]]; then
        make
        # Compile libsafemm_lto
        make lto
    fi

    cd "$ROOT_DIR"
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

    # Download enwik9
    cd "$ROOT_DIR/misc/eval/lzfse_dataset"
    if [[ ! -f enwik9 ]]; then
        wget http://mattmahoney.net/dc/enwik9.zip
        unzip enwik9.zip
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
# parson eval.
#
parson_eval() {
    echo ""
    cd $EVAL_SCRIPTS_DIR
    echo "Evaluating Checked C's performance overhead on parson"
    ./parson_perf.py

    echo "Evaluating Checked C's memory overhead on parson"
    cd mem
    ./parson_run.sh baseline
    ./parson_run.sh checked
    ./parson_mem.py
}

#
# lzfse eval.
#
lzfse_eval() {
    echo ""
    cd $EVAL_SCRIPTS_DIR
    echo "Evaluating Checked C performance overhead on lzfse"
    ./lzfse_run.sh baseline
    ./lzfse_run.sh checked

    echo "Evaluating Checked C memory overhead on lzfse"
    cd mem
    ./lzfse_run.sh baseline
    ./lzfse_run.sh checked

    # Compute results.
    cd ..
    ./lzfse_perf.py
    cd mem
    ./lzfse_mem.py
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
            parson_eval
            ;;
        "lzfse")
            lzfse_eval
            ;;
        *)
            echo "Unknown benchmark names."\
                "Please choose \"olden\", \"thttpd\", \"parson\", or \"lzfse\"."
            ;;
    esac
fi



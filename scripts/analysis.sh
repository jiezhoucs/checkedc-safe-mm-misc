#!/usr/bin/env bash

#
# This script runs analysis pass(es) on selected programs.
#

. common.sh

ANALYSIS_DIR="$ROOT_DIR/analysis"
LLVM_BIN="$ANALYSIS_DIR/llvm-project/build/bin"
CC="$LLVM_BIN/clang"
LIT="$LLVM_BIN/llvm-lit"

LLVM_TESTSUITE_BUILD="$ANALYSIS_DIR/ts-build"
SPEC_BUILD="$LLVM_TESTSUITE_BUILD/External/SPEC"
SPEC_INT_BUILD="$SPEC_BUILD/CINT2017rate"
SPEC_FP_BUILD="$SPEC_BUILD/CFP2017rate"

# SPEC Integer C benchmarks
SPEC_INT_PROGRAMS=(
    "500.perlbench_r"
    "502.gcc_r"
    "505.mcf_r"
    "525.x264_r"
    "557.xz_r"
)

# SPEC FP C benchmarks
SPEC_FP_PROGRAMS=(
    "519.lbm_r"
    "538.imagick_r"
    "544.nab_r"
)


#
# Compile test programs (run pass(es)).
#
compile() {
    for prog in ${SPEC_INT_PROGRAMS[@]}; do
        echo "Compiling $prog"
        cd $SPEC_INT_BUILD/$prog
        make -j8
    done

    for prog in ${SPEC_FP_BUILD[@]}; do
        echo "Compiling $prog"
        cd $SPEC_FP_BUILD/$prog
        make -j8
    done
}

compile

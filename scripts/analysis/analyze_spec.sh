#!/usr/bin/env bash

#
# This script runs analysis pass(es) on SPEC2017 C programs.
#

. common.sh

BUILD_DIR="$ANALYSIS_DIR/ts-build"
SPEC_BUILD="$BUILD_DIR/External/SPEC"
SPEC_INT_BUILD="$SPEC_BUILD/CINT2017rate"
SPEC_FP_BUILD="$SPEC_BUILD/CFP2017rate"

# All SPEC C benchmarks
SPEC_ALL_C=(
    "500.perlbench_r"
    "502.gcc_r"
    "505.mcf_r"
    "525.x264_r"
    "557.xz_r"
    "519.lbm_r"
    "538.imagick_r"
    "544.nab_r"
)

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
# Compile one program
#
compile_one() {
    cd $BUILD_DIR
    echo "Compiling $1"
    make $1 -j8
}


#
# Compile all SPEC C programs
#
compile_all() {
    for prog in ${SPEC_ALL_C[@]}; do
        compile_one $prog
    done
}

#
# Clean all compiled binaries
#
clean() {
    for prog in ${SPEC_INT_PROGRAMS[@]}; do
        echo "removing $prog"
        cd $SPEC_INT_BUILD/$prog
        find . -name "*.o" -delete
        rm -f $prog
    done

    for prog in ${SPEC_FP_PROGRAMS[@]}; do
        echo "removing $prog"
        cd $SPEC_FP_BUILD/$prog
        find . -name "*.o" -delete
        rm -f $prog "$prog.stripped"
    done
}

#
# Entrance of this script
#
if [[ $# == 1 ]]; then
    if [[ $1 == "all" ]]; then
        compile_all
    elif [[ $1 == "clean" ]]; then
        clean
    elif [[ $1 == "ls" ]]; then
        for prog in ${SPEC_ALL_C[@]}; do
            echo $prog
        done
    else
        compile_one $1
    fi
fi

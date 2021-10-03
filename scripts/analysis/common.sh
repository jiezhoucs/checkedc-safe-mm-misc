#!/usr/bin/env bash

# Load the root common.sh script
cd ../
. common.sh

ANALYSIS_DIR="$ROOT_DIR/analysis"
LLVM_BIN="$ANALYSIS_DIR/llvm-project/build/bin"
CC="$LLVM_BIN/clang"
CXX="$LLVM_BIN/clang++"
LD="$LLVM_BIN/ld.lld"
AR="$LLVM_BIN/llvm-ar"
NM="$LLVM_BIN/llvm-nm"
RANLIB="$LLVM_BIN/llvm-ranlib"
LIT="$LLVM_BIN/llvm-lit"
PROGRAMS="$ANALYSIS_DIR/programs"

BREW_LIB="/home/linuxbrew/.linuxbrew/lib"

#
# Analysis passes and library
#
ANALYSIS_LIB="$ANALYSIS_DIR/llvm-project/analysis_lib"
GET_OBJ_SIZE_PASS="-Wl,-mllvm,-get-obj-size"
DYN_STATS_PASS="-Wl,-mllvm,-collect-dyn-stats"

#
# Temporary stats files.
#
STRUCT_SIZE_STAT="/tmp/struct_size.txt"
ANALYSIS_RESULT="/tmp/analysis_result.stat"
# Library functions that take double pointer argument(s).
LIB_FN_STAT="/tmp/lib_fn.stat"

clean_stat_files() {
    rm -f $STRUCT_SIZE_STAT
    rm -f $ANALYSIS_RESULT
    rm -f $LIB_FN_STAT
}

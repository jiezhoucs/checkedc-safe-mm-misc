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

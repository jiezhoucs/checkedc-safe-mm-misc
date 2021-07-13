#!/usr/bin/env bash

# Load the root common.sh script
cd ../
. common.sh

ANALYSIS_DIR="$ROOT_DIR/analysis"
LLVM_BIN="$ANALYSIS_DIR/llvm-project/build/bin"
CC="$LLVM_BIN/clang"
LIT="$LLVM_BIN/llvm-lit"
PROGRAMS="$ANALYSIS_DIR/programs"

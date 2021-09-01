#!/usr/bin/env bash

LLVM_BIN=`realpath ../../../../llvm-vanilla/build/bin`
export CC="$LLVM_BIN/clang"
export AR="$LLVM_BIN/llvm-ar"

make

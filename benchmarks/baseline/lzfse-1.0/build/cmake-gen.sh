#!/usr/bin/env bash

ROOT_DIR=`realpath ../../../../../`
BUILD_DIR=`realpath .`
LLVM_BIN="$ROOT_DIR/llvm-vanilla/build/bin"
CC="$LLVM_BIN/clang"
CFLAGS="-O3"

rm -rf CMakeCache.txt

cmake .. -G "Unix Makefiles"                                                              \
      -DCMAKE_C_COMPILER="$CC"                                                 \
      -DCMAKE_C_FLAGS="$CFLAGS"                                                \
      -DCMAKE_INSTALL_PREFIX="$BUILD_DIR"                                      \

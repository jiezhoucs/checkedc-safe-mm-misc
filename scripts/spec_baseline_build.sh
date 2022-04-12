#!/usr/bin/env bash

. common.sh

LLVM_DIR="$ROOT_DIR/llvm-vanilla"
LLVM_BIN_DIR="$LLVM_DIR/build/bin"
TEST_SUITE_SRC="$LLVM_DIR/llvm/projects/test-suite"
TEST_SUITE_BUILD="$ROOT_DIR/benchmark-build/spec/baseline"
SPEC_SRC="/home/jie/projects/common/spec-cpu2006"

CC="$LLVM_BIN_DIR/clang"
# LTO has almost no improvement on 429.mcf.
# CFLAGS="-flto"
# LDFLAGS="-fuse-ld=lld"

if [[ ! -d $TEST_SUITE_BUILD ]]; then
    mkdir -p $TEST_SUITE_BUILD
fi

cd $TEST_SUITE_BUILD
rm -rf CMakeCache.txt

cmake -DCMAKE_C_COMPILER="$CC"                      \
      -DCMAKE_C_FLAGS="$CFLAGS"                     \
      -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS"           \
      -DCMAKE_BUILD_TYPE=Release                    \
      -DTEST_SUITE_SPEC2006_ROOT="$SPEC_SRC"        \
      -DTEST_SUITE_RUN_TYPE=ref                     \
      "$TEST_SUITE_SRC"

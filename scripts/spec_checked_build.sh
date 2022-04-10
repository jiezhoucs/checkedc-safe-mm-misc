#!/usr/bin/env bash

. common.sh

TEST_SUITE_SRC="$LLVM_SRC/projects/test-suite"
TEST_SUITE_BUILD="$ROOT_DIR/benchmark-build/spec/checked"
SPEC_SRC="/home/jie/projects/common/cpu2017"

# CFLAGS="-flto"
# LDFLAGS="-fuse-ld=lld"

LDFLAGS="$CHECKEDC_LIB/libsafemm.a -lstdc++ $CHECKEDC_LIB/libporting.a"

if [[ ! -d $TEST_SUITE_BUILD ]]; then
    mkdir -p $TEST_SUITE_BUILD
fi

cd $TEST_SUITE_BUILD
rm -rf CMakeCache.txt

cmake -DCMAKE_C_COMPILER="$CC"                      \
      -DCMAKE_C_FLAGS="$CFLAGS"                     \
      -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS"           \
      -DCMAKE_BUILD_TYPE=Release                    \
      -DTEST_SUITE_SPEC2017_ROOT="$SPEC_SRC"        \
      -DTEST_SUITE_RUN_TYPE=train                   \
      "$TEST_SUITE_SRC"

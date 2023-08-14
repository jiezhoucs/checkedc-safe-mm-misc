#!/usr/bin/env bash

. common.sh

ANALYSIS_LIB="$ANALYSIS_DIR/llvm-project/analysis_lib"
CFLAGS="-flto"
LDFLAGS="-fuse-ld=lld -Wl,-mllvm,-collect-dyn-stats -lstdc++ $ANALYSIS_LIB/libanalysis.o"

TEST_SUITE_SRC="$ANALYSIS_DIR/llvm-project/llvm/projects/test-suite"
TEST_SUITE_BUILD="$ANALYSIS_DIR/ts-build"
SPEC_SRC="/home/jie/projects/common/cpu2017"

cd $TEST_SUITE_BUILD
rm -rf CMakeCache.txt

cmake -DCMAKE_C_COMPILER="$CC"                                                 \
      -DCMAKE_CXX_COMPILER="$CXX"                                              \
      -DCMAKE_C_FLAGS="$CFLAGS"                                                \
      -DCMAKE_BUILD_TYPE=Release                                               \
      -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS"                                      \
      -DTEST_SUITE_SPEC2017_ROOT="$SPEC_SRC"                                   \
      -DTEST_SUITE_RUN_TYPE=ref                                              \
      "$TEST_SUITE_SRC"


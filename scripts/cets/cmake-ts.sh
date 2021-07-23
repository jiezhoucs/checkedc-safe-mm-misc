#!/bin/bash

#
# This scripts generates Makefiles for the original llvm test-suite
#


# Load the common paths and variables.
. common.sh

TESTSUITE_DIR="$TESTS_DIR/test-suite-cets"
# THe lld in the CFLAGS is to make clang invoke lld; the default ld cannot link
# CETS libraries correctly
CFLAGS="-O3 -fuse-ld=lld -mllvm -enable-softboundcets -mllvm -softboundcets_disable_spatial_safety"
LIBS="-lm -lrt"
LDFLAGS="$CETS_LIB/libsoftboundcets_rt.a -L$CETS_LIB -Wl,-rpath,$CETS_LIB $LIBS"

# Go to the build directory. Create one if it does not exist.
[[ -d $LLVM_TS_BUILD ]] || mkdir -p $LLVM_TS_BUILD
cd "$LLVM_TS_BUILD"

rm -rf CMakeCache.txt

cmake -DCMAKE_C_COMPILER="$CETS_CC"                                            \
      -DCMAKE_C_FLAGS="$CFLAGS"                                                \
      -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS"                                      \
      -C"$TESTSUITE_DIR"/cmake/caches/O3.cmake                                 \
      -DLARGE_PROBLEM_SIZE=1                                                   \
      "$TESTSUITE_DIR"

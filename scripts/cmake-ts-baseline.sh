#!/bin/bash

#
# This scripts generates Makefiles for the original llvm test-suite
#


# Load the common paths and variables.
. common.sh

TESTSUITE_ORIGIN_DIR="$TESTS_DIR/test-suite-baseline"
TESTSUITE_ORIGIN_BUILD_DIR="$TESTS_DIR/ts-build-baseline"

# Use lld to link the libsafemm.
LDFLAGS="-fuse-ld=lld"

# Go to the build directory. Create one if it does not exist.
[[ -d $TESTSUITE_ORIGIN_BUILD_DIR ]] || mkdir -p $TESTSUITE_ORIGIN_BUILD_DIR
cd "$TESTSUITE_ORIGIN_BUILD_DIR"
rm -rf CMakeCache.txt

if [[ $1 == "lto" ]]; then
    CFLAGS="-flto $CFLAGS"
fi

cmake -DCMAKE_C_COMPILER="$CC_VANILLA"                                         \
      -DCMAKE_C_FLAGS="$CFLAGS"                                                \
      -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS"                                      \
      -C"$TESTSUITE_ORIGIN_DIR"/cmake/caches/O3.cmake                          \
      -DEXTRA_LARGE_PROBLEM_SIZE=1                                             \
      "$TESTSUITE_ORIGIN_DIR"

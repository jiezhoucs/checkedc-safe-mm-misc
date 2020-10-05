#!/bin/bash

#
# This scripts generates Makefiles for the original llvm test-suite
#


# Load the common paths and variables.
. common.sh

TESTSUITE_ORIGIN_DIR="$TESTS_DIR/test-suite-origin"
TESTSUITE_ORIGIN_BUILD_DIR="$TESTS_DIR/ts-build-origin"

# Go to the build directory. Create one if it does not exist.
[[ -d $TESTSUITE_ORIGIN_BUILD_DIR ]] || mkdir -p $TESTSUITE_BUILD_DIR
cd "$TESTSUITE_ORIGIN_BUILD_DIR"
rm -rf CMakeCache.txt

cmake -DCMAKE_C_COMPILER="$CC"                                                 \
      -C"$TESTSUITE_ORIGIN_DIR"/cmake/caches/O3.cmake                          \
      "$TESTSUITE_ORIGIN_DIR"

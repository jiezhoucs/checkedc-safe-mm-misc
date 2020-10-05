#!/bin/bash

#
# This scripts generates Makefiles for the llvm test-suite for the modified
# Checked C version.
#

# Load the common paths and variables.
. common.sh

# Go to the build directory. Create one if it does not exist.
[[ -d $TESTSUITE_BUILD_DIR ]] || mkdir -p $TESTSUITE_BUILD_DIR
cd "$TESTSUITE_BUILD_DIR"
rm -rf CMakeCache.txt

cmake -DCMAKE_C_COMPILER=$CC                                 \
      -DCMAKE_PREFIX_PATH="$CHECKEDC_INC;$CHECKEDC_LIB"      \
      -C"$TESTSUITE_DIR"/cmake/caches/O3.cmake               \
      "$TESTSUITE_DIR"

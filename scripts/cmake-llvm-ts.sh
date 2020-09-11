#!/bin/bash

#
# This scripts generates Makefiles for the llvm test-suite.
#

# Load the common paths and variables.
. common.sh

# Go to the build directory. Create one if it does not exist.
[[ -d $LLVM_TESTSUITE_BUILD_DIR ]] || mkdir -p $LLVM_TESTSUITE_BUILD_DIR
cd "$LLVM_TESTSUITE_BUILD_DIR"
rm -rf CMakeCache.txt

cmake -DCMAKE_C_COMPILER=$CC                                 \
      -DCMAKE_PREFIX_PATH="$CHECKEDC_INC;$CHECKEDC_LIB"      \
      -C"$LLVM_TESTSUITE_DIR"/cmake/caches/O3.cmake          \
      "$LLVM_TESTSUITE_DIR"

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

#
# Choose linker. Unfortunately lld seems to be not working on MacOS.
#
if [[ `uname` == "Linux" ]]; then
    LDFLAGS="-fuse-ld=lld"
fi

if [[ $1 == "lto" ]]; then
    CFLAGS="-flto $CFLAGS"
    LDFLAGS="$LDFLAGS $CHECKEDC_LIB/libsafemm_lto.a"
else
    LDFLAGS="$LDFLAGS $CHECKEDC_LIB/libsafemm.a"
fi

cmake -DCMAKE_C_COMPILER="$CC"                                                 \
      -DCMAKE_C_FLAGS="$CFLAGS"                                                \
      -DCMAKE_CXX_FLAGS="-mllvm -checkedc-init=false"                          \
      -DCMAKE_PREFIX_PATH="$CHECKEDC_INC;$CHECKEDC_LIB"                        \
      -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS"                                      \
      -C"$TESTSUITE_DIR"/cmake/caches/O3.cmake                                 \
      -DEXTRA_LARGE_PROBLEM_SIZE=1                                             \
      "$TESTSUITE_DIR"

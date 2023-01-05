#!/usr/bin/env bash

ROOT_DIR=`realpath ../../../../../`
MISC_DIR="$ROOT_DIR/misc"
BUILD_DIR=`realpath .`
LLVM_BIN="$ROOT_DIR/build/bin"
CC="$LLVM_BIN/clang"
CFLAGS="-O3 -I$MISC_DIR/include"
export LDFLAGS="-fuse-ld=lld -L$MISC_DIR/lib -lsafemm"  # CMAKE_LINKER_EXE does not work.

rm -rf CMakeCache.txt

cmake .. -G "Unix Makefiles"                                                   \
      -DCMAKE_C_COMPILER="$CC"                                                 \
      -DCMAKE_C_FLAGS="$CFLAGS"                                                \
      -DCMAKE_INSTALL_PREFIX="$BUILD_DIR"                                      \

#
# Add targets in the Makefile for simple testing
#

sed -i '$aencode:\n\t./lzfse \-v \-encode \-i enwik8 \-o enwik8_compressed' Makefile
echo >> Makefile
sed -i '$adecode:\n\t./lzfse \-v \-decode \-i enwik8_compressed \-o enwik8_origin' Makefile

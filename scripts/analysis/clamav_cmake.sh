#!/usr/bin/env bash

ROOT_DIR=`realpath ../../../`
LLVM_BIN="$ROOT_DIR/llvm-project/build/bin"
BUILD_DIR=`realpath .`
CC="$LLVM_BIN/clang"
CXX="$LLVM_BIN/clang++"
CFLAGS="-flto"
LDFLAGS="-fuse-ld=lld"

#
# Passes
#
GET_OBJ_SIZE_PASS="-Wl,-mllvm,-get-obj-size"
MARSHAL_ARRAY_PASS="-Wl,-mllvm,-marshal-array-size"

LDFLAGS="$LDFLAGS $MARSHAL_ARRAY_PASS"

cd $BUILD_DIR
rm -rf CMakeCache.txt

cmake .. -G Ninja                                                              \
      -D CMAKE_BUILD_TYPE="Release"                                            \
      -D CMAKE_C_COMPILER="$CC"                                                \
      -D CMAKE_CXX_COMPILER="$CXX"                                             \
      -D CMAKE_C_FLAGS="$CFLAGS"                                               \
      -D CMAKE_CXX_FLAGS="$CFLAGS"                                             \
      -D CMAKE_EXE_LINKER_FLAGS="$LDFLAGS"                                     \
      -D CMAKE_INSTALL_PREFIX="$BUILD_DIR"                                     \
      -D ENABLE_MILTER=OFF                                                     \
      -D ENABLE_SYSTEMD=OFF                                                    \

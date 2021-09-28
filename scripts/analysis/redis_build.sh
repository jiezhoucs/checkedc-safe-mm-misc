#!/usr/bin/env bash

#
# This scripts configures and builds redis-6.2.5
#

. common.sh

SRC_DIR="$PROGRAMS/redis-6.2.5"
BUILD_DIR="$SRC_DIR/build"

CC="$CC"
CXX="$CXX"
CFLAGS="-flto"
CXXFLAGS="-flto"
LD="$CC"
LDFLAGS="-fuse-ld=lld"
LDFLAGS="$LDFLAGS -lstdc++ $DYN_STATS_PASS $ANALYSIS_LIB/libanalysis.o"
# LDFLAGS="$LDFLAGS $GET_OBJ_SIZE_PASS"
AR="$AR"
NM="$NM"

cd $SRC_DIR

make clean
make CC="$CC" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" -j8
make PREFIX="$BUILD_DIR" install

rm -f /tmp/analysis_result.txt

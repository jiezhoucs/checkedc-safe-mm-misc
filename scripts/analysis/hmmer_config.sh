#!/usr/bin/env bash

#
# This scripts generates build files ,and compiles hmmer-3.3.2.
#

. common.sh

SRC_DIR="$PROGRAMS/hmmer-3.3.2"
BUILD_DIR="$SRC_DIR/build"

export CC="$CC"
export CFLAGS="-flto"
export LD="$CC"
export LDFLAGS="-fuse-ld=lld -lstdc++ $DYN_STATS_PASS $ANALYSIS_LIB/libanalysis.o"
export AR="$AR"
export NM="$NM"

cd $SRC_DIR
mkdir -p build

#
# Clean old files
#
rm -f /tmp/analysis_result.txt
make clean

#
# Configure (assuming the configure file is there. In case it is not generated,
# run autoconf to create one.
#
./configure --prefix="$BUILD_DIR"

make -j8

rm -f /tmp/analysis_result.txt

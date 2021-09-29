#!/usr/bin/env bash

#
# This scripts builds sqlite-3.37.0
#

. common.sh

SRC_DIR="$PROGRAMS/sqlite-3.37.0"
BUILD_DIR="$SRC_DIR/build"

#
# The "Var=Val" setting following configure does not work. We need export.
#
export CC="$CC"
export CFLAGS="-flto"
LDFLAGS="-fuse-ld=lld"
export LDFLAGS="$LDFLAGS $DYN_STATS_PASS -lstdc++ $ANALYSIS_LIB/libanalysis.o"
# export LDFLAGS="$LDFLAGS $GET_OBJ_SIZE_PASS"
export AR="$AR"
export NM="$NM"
export RANLIB="$RANLIB"

# Remove existing data file.
rm -f /tmp/analysis_result.txt

#
# Start to build
#
cd $BUILD_DIR
make clean
../configure
make -j8

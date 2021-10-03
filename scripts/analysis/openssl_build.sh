#!/usr/bin/env bash

#
# This scripts configures openssl-3.0.0.
#
# For testing, do "make test"
#

. common.sh

SRC_DIR="$PROGRAMS/openssl-3.0.0"
BUILD_DIR="$SRC_DIR/build"

export CC="$CC"
export CFLAGS="-flto"
LDFLAGS="-fuse-ld=lld"
export LDFLAGS="$LDFLAGS $DYN_STATS_PASS -lstdc++ $ANALYSIS_LIB/libanalysis.o"
# export LDFLAGS="$LDFLAGS $GET_OBJ_SIZE_PASS"
export AR="$AR"
export NM="$NM"
export RANLIB="$RANLIB"

#
# Start to build
#
cd $SRC_DIR
if [[ -f Makefile ]]; then
    make clean
fi

./config --prefix="$BUILD_DIR"  \
         -no-shared

#
# Remove existing data file.
#
clean_stat_files

#
# Compile
#
if [[ $1 == "make" ]]; then
    make -j8
    make install -j8
fi

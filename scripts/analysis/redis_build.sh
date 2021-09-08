#!/usr/bin/env bash

#
# This scripts configures the compiling files of redis-6.2.5
#

. common.sh

SRC_DIR="$PROGRAMS/redis-6.2.5"
BUILD_DIR="$SRC_DIR/build"

export CC="$CC"
export CXX="$CXX"
export CFLAGS="-flto"
export CXXFLAGS="-flto"
export LD="$CC"
export LDFLAGS="-fuse-ld=lld $MARSHAL_ARRAY_PASS"
export AR="$AR"
export NM="$NM"

cd $SRC_DIR

make -j8

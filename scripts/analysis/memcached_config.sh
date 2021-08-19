#!/usr/bin/env bash

#
# This scripts configures the compiling files of memcached.
#

. common.sh

SRC_DIR="$PROGRAMS/memcached"

export CC="$CC"
export CFLAGS="-flto"
export LDFLAGS="-fuse-ld=lld -Wl,-mllvm,-get-obj-size"
export AR="$AR"
export NM="$NM"

cd $SRC_DIR
./configure --prefix="$SRC_DIR/build"                   \


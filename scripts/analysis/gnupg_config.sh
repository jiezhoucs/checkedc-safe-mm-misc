#!/usr/bin/env bash

#
# This scripts configures the compiling files of PHP-8.0.9
#

. common.sh

# GNUPG-2.3.1 cannot compile because some dependency library are too old on
# Jie Zhou's machine.
SRC_DIR="$PROGRAMS/gnupg"

export CC="$CC"
export CXX="$CXX"
export CFLAGS="-flto"
export CXXFLAGS="-flto"
export LD="$CC"
export LDFLAGS="-fuse-ld=lld -Wl,-mllvm,-get-obj-size"
export AR="$AR"
export NM="$NM"

cd $SRC_DIR
./configure --prefix="$SRC_DIR/build"                   \
    --enable-maintainer-mode \
            CC="$CC"                                    \
            CXX="$CXX"                                  \
            LD="$CC"                                    \
            CFLAGS="$CFLAGS"                            \
            CXXFLAGS="$CXXFLAGS"                        \
            LDFLAGS="$LDFLAGS"

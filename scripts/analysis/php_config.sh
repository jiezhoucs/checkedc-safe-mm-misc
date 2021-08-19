#!/usr/bin/env bash

#
# This scripts configures the compiling files of PHP-8.0.9
#

. common.sh

SRC_DIR="$PROGRAMS/php-src"

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
            CC="$CC"                                    \
            CXX="$CXX"                                  \
            LD="$CC"                                    \
            CFLAGS="$CFLAGS"                            \
            CXXFLAGS="$CXXFLAGS"                        \
            LDFLAGS="$LDFLAGS"

#
# For some unkown reason, $LDFLAGS is not written to the generated Makefile
# (but -flto is there for $CFLAGS. Here we manually set the LDFALGS in the
# Makefile. Also, for another unknown reason, the $LDFLAGS cannot have
# "-fuse-ld=lld"; it would otherwise reports a "invalid linker name in argument '-fuse-ld=lld -Wl,-mllvm,-get-obj-size'"
# error during linking.
#
sed -i "1iLDFLAGS=-Wl,-mllvm,-get-obj-size" Makefile

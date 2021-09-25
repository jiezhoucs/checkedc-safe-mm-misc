#!/usr/bin/env bash

#
# This scripts configures the compiling files of nginx.
#

. common.sh

SRC_DIR="$PROGRAMS/nginx-1.21.1"
BUILD_DIR="$SRC_DIR/build"

CC="$CC"
CFLAGS="-flto"
LDFLAGS="-fuse-ld=lld $DYN_STATS_PASS -lstdc++ $ANALYSIS_LIB/libanalysis.o"

cd "$SRC_DIR"
./configure --prefix="$BUILD_DIR"                                              \
            --builddir="$BUILD_DIR"                                            \
            --with-cc="$CC"                                                    \
            --with-cc-opt="$CFLAGS"                                            \
            --with-ld-opt="$LDFLAGS"                                           \
            --without-http_rewrite_module                                      \

# Compile it if the first argument is "make".
if [[ $1 == "make" ]]; then
    make -j$PARA_LEVEL
    make install
fi

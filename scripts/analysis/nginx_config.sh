#!/usr/bin/env bash

#
# This scripts configures the compiling files of nginx.
#

. common.sh

SRC_DIR="$PROGRAMS/nginx-1.21.1"

CC="$CC"
CFLAGS="-flto"
LDFLAGS="-fuse-ld=lld -Wl,-mllvm,-get-obj-size"

cd "$SRC_DIR"
./configure --prefix="$SRC_DIR/build"                                          \
            --with-cc="$CC"                                                    \
            --with-cc-opt="$CFLAGS"                                            \
            --with-ld-opt="$LDFLAGS"                                           \
            --without-http_rewrite_module                                      \

# Compile it if the first argument is "make".
if [[ $1 == "make" ]]; then
    make -j$PARA_LEVEL
fi

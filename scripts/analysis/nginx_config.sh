#!/usr/bin/env bash

#
# This scripts configures and invokes the configure file of nginx
#

. common.sh

SRC_DIR="$PROGRAMS/nginx-1.21.1"

export CC="$CC"
export CFLAGS="-mllvm -get-obj-size"

cd "$SRC_DIR"
./configure --prefix="$SRC_DIR/build"                                          \
            --without-http_rewrite_module

# Compile it if the first argument is "make".
if [[ $1 == "make" ]]; then
    make -j$PARA_LEVEL
fi

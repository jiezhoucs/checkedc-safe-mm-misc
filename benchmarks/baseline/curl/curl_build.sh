#!/usr/bin/env bash

#
# This scripts configures the Makefiles for curl-7.79.1.
# This script is supposed to be invoked in its directory.
#
# To run built-in tests, do "make test -jx".
#

ROOT_DIR=$(realpath ../../../../)
MISC_DIR=$ROOT_DIR/misc

CC=$ROOT_DIR/llvm-vanilla/build/bin/clang
CFLAGS="-O3 -Wall"

SRC_DIR=`realpath .`
BUILD_DIR="$SRC_DIR/build"

#
# Run configure to create build files.
#
config_configure() {
    if [[ ! -f "configure" ]]; then
        autoreconf -fi
    fi

    cd $SRC_DIR
    if [[ ! -d $BUILD_DIR ]]; then
        mkdir $BUILD_DIR
    fi

    if [[ -f Makefile ]]; then
        make -k clean
    fi

    ./configure  --prefix="$BUILD_DIR"                                         \
                 --disable-shared                                              \
                 --disable-threaded-resolver                                   \
                 --with-ssl                                                    \
                 CC="$CC"                                                      \
                 CFLAGS="$CFLAGS"
}

#
# Entrance of this script.
#
config_configure

if [[ $1 == "make" ]]; then
    make -j8
fi

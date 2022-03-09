#!/usr/bin/env bash

#
# This scripts configures the Makefiles for curl-7.79.1.
#
# To run built-in tests, do "make test -jx".
#

ROOT_DIR=$(realpath ../../../../)
MISC_DIR=$ROOT_DIR/misc

CC=$ROOT_DIR/build/bin/clang
CFLAGS="-O3 -Wall"
CFLAGS="$CFLAGS -I$MISC_DIR/include"
LDFLAGS="-L$MISC_DIR/lib -lsafemm -ldebug"

SRC_DIR=`realpath .`
BUILD_DIR="$SRC_DIR/build"
OPENSSL_DIR=/home/linuxbrew/.linuxbrew/Cellar/openssl@1.1/1.1.1l_1

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
                 --with-openssl="$OPENSSL_DIR"                                 \
                 CC="$CC"                                                      \
                 CFLAGS="$CFLAGS"                                              \
                 LD="$CC"                                                      \
                 LDFLAGS="$LDFLAGS"                                            \
}

#
# Entrance of this script.
#
# clean_stat_files

config_configure

if [[ $1 == "make" ]]; then
    make -j8
fi

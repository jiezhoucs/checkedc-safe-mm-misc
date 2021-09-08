#!/usr/bin/env  bash

#
# This script generates the Makefile for thttpd.
#
# $1 - (optional) "baseline" for baseline, otherwise for the checked version.
# $2 - (optional) "make" to compile thttpd.
#

# load common directory paths and variables
. common.sh

BUILD_DIR=$ROOT_DIR/benchmark-build/thttpd/cets

#
# Check if it's for the baseline or the checked version.
SRC_DIR=$MISC_DIR/benchmarks/cets/thttpd-2.29
CC="$CETS_CC"
CFLAGS="-mllvm -enable-softboundcets -mllvm -softboundcets_disable_spatial_safety"

#
# Run configure.sh to generate Makefile
#
configure() {
    cd $SRC_DIR

    export CC="$CC"

    rm -f config.cache

    if [[ ! -f "$BUILD_DIR" ]]; then
        mkdir -p "$BUILD_DIR"
    fi
    ./configure --prefix="$BUILD_DIR"

    # link the CETS library
    sed -i "s|^CFLAGS =|& $CFLAGS|g" Makefile
    sed -i "s|^LDFLAGS =|& $CETS_LIB/libsoftboundcets_rt.a|g" Makefile
    sed -i "s|\-lcrypt $|& \-lm -lrt|g" Makefile

    # mkdir a directory for man page
    if [[ ! -f "$BUILD_DIR/man/man1" ]]; then
        mkdir -p "BUILD_DIR/man/man1"
    fi
}

#
# Compile thttpd
#
compile() {
    cd $SRC_DIR
    if [[ -f "Makefile" ]]; then
        make -j4
    else
        echo "Makefile not found. Please first generate the Makefile."
        exit
    fi
}

#
# Entrance of this script
#
configure

#
# Note that installing the compiled and related files requires to run as root.
# 07/24/2021: Jie Zhou: I forgot the exact reason for this.
#
if [[ $2 == "make" ]]; then
    compile
fi

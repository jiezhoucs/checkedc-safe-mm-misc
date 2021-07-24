#!/usr/bin/env  bash

#
# This script generates the Makefile for thttpd.
#
# $1 - (optional) "baseline" for baseline, otherwise for the checked version.
# $2 - (optional) "make" to compile thttpd.
#

# load common directory paths and variables
. common.sh

BUILD_DIR=$ROOT_DIR/benchmark-build/thttpd

#
# Check if it's for the baseline or the checked version.
if [[ $1 == "baseline" ]]; then
    SRC_DIR=$MISC_DIR/benchmarks/baseline/thttpd-2.29
    BUILD_DIR=$BUILD_DIR/baseline
    CC="$CC_VANILLA"
else
    SRC_DIR=$MISC_DIR/benchmarks/checked/thttpd-2.29
    BUILD_DIR=$BUILD_DIR/checked
fi

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

    # link the mm_safe library
    if [[ $1 != "baseline" ]]; then
        sed -i "s|^LDFLAGS =|& \-L../../../lib|g" Makefile
        sed -i "s|\-lcrypt $|& \-lsafemm|g" Makefile
    fi

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
configure $1

#
# Note that installing the compiled and related files requires to run as root.
# 07/24/2021: Jie Zhou: I forgot the exact reason for this.
#
if [[ $2 == "make" ]]; then
    compile
fi

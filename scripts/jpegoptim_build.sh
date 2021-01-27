#!/usr/bin/env  bash

#
# This script generates the Makefile for jpegoptim.
#
# $1 - (optional) "baseline" for baseline, otherwise for the checked version
# $2 - (optional) "make" to compile jpegoptim
#

# load common directory paths and variables
. common.sh


#
# Decide if it's for the baseline or the checked version.
#
if [[ $1 == "baseline" ]]; then
    SRC_DIR=$MISC_DIR/benchmarks/baseline/jpegoptim
    BUILD_DIR=$ROOT_DIR/benchmark-build/baseline/jpegoptim
else
    SRC_DIR=$MISC_DIR/benchmarks/checked/jpegoptim
    BUILD_DIR=$ROOT_DIR/benchmark-build/checked/jpegoptim
fi

#
# Run configure.sh to generate Makefile
#
configure() {
    cd $SRC_DIR

    # set up compiler
    export CC="$CC"
    if [[ $OS == "Darwin" ]]; then
        export LDFLAGS="-Xlinker -syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"
        LIBJPEG_DIR="/usr/local/Cellar/jpeg/9d"
    else
        LIBJPEG_DIR=$(dirname `ldconfig -p | grep libjpeg | cut -d '>' -f2`)
    fi

    if [[ ! -f "$BUILD_DIR" ]]; then
        mkdir -p "$BUILD_DIR"
    fi
    ./configure --prefix="$BUILD_DIR"                                          \
                --with-libjpeg="$LIBJPEG_DIR"
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


if [[ $2 == "make" ]]; then
    compile
fi

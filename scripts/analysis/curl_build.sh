#!/usr/bin/env bash

#
# This scripts configures the Makefiles for curl-7.79.1.
#
# To run built-in tests, do "make test -jx".
#

. common.sh

SRC_DIR="$PROGRAMS/curl-7.79.1"
BUILD_DIR="$SRC_DIR/build"
OPENSSL_DIR=/home/linuxbrew/.linuxbrew/Cellar/openssl@1.1/1.1.1l

CFLAGS="-flto"
LDFLAGS="-fuse-ld=lld"
LDFLAGS="$LDFLAGS $DYN_STATS_PASS -lstdc++ $ANALYSIS_LIB/libanalysis.o"
# LDFLAGS="$LDFLAGS $GET_OBJ_SIZE_PASS"

#
# Set the options for cmake.
#
# We do not use it for now because it does not work. For curl-7.79.1, there
# are header file not found errors; and for curl-7.70.0, users are reminded
# by the cmake files that cmake for curl is poorly maintained.
#
config_cmake() {
    cd $SRC_DIR
    if [[ ! -d $BUILD_DIR ]]; then
        mkdir $BUILD_DIR
    fi
    cd $BUILD_DIR
    rm -rf CMakeCache.txt

    cmake -G "Unix Makefiles"                                                  \
          -DCMAKE_C_COMPILER="$CC"                                             \
          -DCMAKE_C_FLAGS="$CFLAGS"                                            \
          -DCMAKE_LINKER="$CC"                                                 \
          -DCMAKE_EXE_LINKER_FLAGS="$LDFLAGS"                                  \
          -DCMAKE_AR="$AR"                                                     \
          -DCMAKE_RANLIB="$RANLIB"                                             \
          -DCMAKE_NM="$NM"                                                     \
          -DCMAKE_INSTALL_PREFIX="$BUILD_DIR"                                  \
          -DENABLE_THREADED_RESOLVER=OFF                                       \
          -DBUILD_SHARED_LIBS=OFF                                              \
          $SRC_DIR
}

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
                 AR="$AR"                                                      \
                 RANLIB="$RANLIB"                                              \
                 NM="$NM"
}

#
# Entrance of this script.
#
clean_stat_files

config_configure

if [[ $1 == "make" ]]; then
    make -j8
fi

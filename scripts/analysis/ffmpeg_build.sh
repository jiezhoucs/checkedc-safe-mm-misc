#!/usr/bin/env bash

#
# This scripts configures ffmpeg-n4.1.7
#
# Test: "make check" or
# "make fate" (https://ffmpeg.org/fate.html)
#

. common.sh

SRC_DIR="$PROGRAMS/FFmpeg-n4.1.7"
BUILD_DIR="$SRC_DIR/build"

CFLAGS="-flto"
LDFLAGS="-fuse-ld=lld"
LDFLAGS="$LDFLAGS $DYN_STATS_PASS -lstdc++ $ANALYSIS_LIB/libanalysis.o"
# LDFLAGS="$LDFLAGS $GET_OBJ_SIZE_PASS"

# Remove existing data file.
rm -f /tmp/struct_size.txt
rm -f /tmp/analysis_result.txt

#
# Start to configure
#
cd $SRC_DIR
mkdir -p $BUILD_DIR
if [[ -f Makefile ]]; then
    make -k clean
fi

./configure --prefix="$BUILD_DIR"                                              \
            --cc="$CC"                                                         \
            --extra-cflags="$CFLAGS"                                           \
            --extra-ldflags="$LDFLAGS"                                         \
            --ar="$AR"                                                         \
            --nm="$NM"                                                         \
            --ranlib="$RANLIB"                                                 \
            --samples=fate-suite

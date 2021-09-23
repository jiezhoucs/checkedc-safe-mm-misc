#!/usr/bin/env bash

#
# This scripts configures the compiling files of gnupg-2.2.20.
#
# Jie Zhou: GNUPG-2.3.1 cannot compile because some dependency library are
# too old on my machine.
#

. common.sh

SRC_DIR="$PROGRAMS/gnupg-2.2.20"

export CC="$CC"
export CXX="$CXX"
export CFLAGS="-flto"
export CXXFLAGS="-flto"
export LD="$CC"
export LDFLAGS="-fuse-ld=lld $DYN_STATS_PASS -lstdc++ $ANALYSIS_LIB/libanalysis.o"
export AR="$AR"
export NM="$NM"

cd $SRC_DIR
./configure --prefix="$SRC_DIR/build"                   \
    --enable-maintainer-mode \
            CC="$CC"                                    \
            CXX="$CXX"                                  \
            LD="$CC"                                    \
            CFLAGS="$CFLAGS"                            \
            CXXFLAGS="$CXXFLAGS"                        \
            LDFLAGS="$LDFLAGS"

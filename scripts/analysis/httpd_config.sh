#!/usr/bin/env bash

#
# This scripts configures the compiling files of httpd.
#

. common.sh

SRC_DIR="$PROGRAMS/httpd-2.4.48"

export CC="$CC"
export CFLAGS="-flto"
export LD="$CC"
export LDFLAGS="-fuse-ld=lld -Wl,-mllvm,-get-obj-size"
export AR="$AR"
export NM="$NM"

##
## Back up ld, ar, and nm
##
LD_ORIGIN=$(file $(which ld) | cut -d ' ' -f5)
AR_ORIGIN=$(file $(which ar) | cut -d ' ' -f5)
NM_ORIGIN=$(file $(which nm) | cut -d ' ' -f5)

##
## Set ld, ar, and nm to the llvm versions
##
LD_SYM=`which ld`
AR_SYM=`which ar`
NM_SYM=`which nm`
rm -f $LD_SYM
rm -f $AR_SYM
rm -f $NM_SYM
ln -s $LLVM_BIN/ld.lld $LD_SYM
ln -s $LLVM_BIN/llvm-ar $AR_SYM
ln -s $LLVM_BIN/llvm-nm $NM_SYM

cd $SRC_DIR

make clean

./configure --prefix="$SRC_DIR/build"                      \

#
# Restore ld, ar, and nm
#
restore() {
    rm -f $LD_SYM
    ln -s $LD_ORIGIN $LD_SYM
    rm -f $AR_SYM
    ln -s $AR_ORIGIN $AR_SYM
    rm -f $NM_SYM
    ln -s $NM_ORIGIN $NM_SYM
}

if [[ $1 == "make" ]]; then
    make -j8
    restore
elif [[ $1 == "restore" ]]; then
    restore
fi

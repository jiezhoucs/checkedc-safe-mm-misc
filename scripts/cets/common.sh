#!/bin/bash

cd ..
. common.sh

# CETS related
CETS_ROOT=$ROOT_DIR/cets
CETS_BIN=$CETS_ROOT/build/bin
CETS_LIB=$CETS_ROOT/llvm/libsoftboundcets
CETS_CC=$CETS_BIN/clang
CETS_LINKER=$CETS_BIN/lld
CETS_LIT="$CETS_BIN/llvm-lit"
LLVM_TS_BUILD=$TESTS_DIR/ts-build-cets
DATA_DIR="$DATA_DIR/olden/cets"

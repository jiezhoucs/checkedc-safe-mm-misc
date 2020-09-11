#!/bin/bash

#
# project paths
#
ROOT_DIR=`realpath ../../`
MISC_DIR="$ROOT_DIR/misc"
LLVM_BIN_DIR="$ROOT_DIR/build/bin"
LIT="$LLVM_BIN_DIR/llvm-lit"
TESTS_DIR="$ROOT_DIR/tests"
LLVM_TESTSUITE_DIR="$TESTS_DIR/test-suite"
LLVM_TESTSUITE_BUILD_DIR="$TESTS_DIR/ts-build"
OLDEN_DIR="$LLVM_TESTSUITE_DIR/MultiSource/Benchmarks/Olden"
PTRDIST_DIR="$LLVM_TESTSUITE_DIR/MultiSource/Benchmarks/Ptrdist"
DATA_DIR="$MISC_DIR/data"

#
# Compiler related
#
CC=$LLVM_BIN_DIR/clang
CXX=$LLVM_BIN_DIR/clang++
CHECKEDC_INC=$MISC_DIR/include
CHECKEDC_LIB=$MISC_DIR/lib


#
# Others
#
OS=`uname`

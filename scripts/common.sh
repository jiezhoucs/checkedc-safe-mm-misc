#!/bin/bash

#
# project paths
#
ROOT_DIR=`realpath ../../`
MISC_DIR="$ROOT_DIR/misc"
LLVM_BIN_DIR="$ROOT_DIR/build/bin"
LIT="$LLVM_BIN_DIR/llvm-lit"
TESTS_DIR="$ROOT_DIR/tests"
LLVM_TESTSUITE_DIR="$ROOT_DIR/tests/test-suite"
OLDEN_DIR="$LLVM_TESTSUITE_DIR/MultiSource/Benchmarks/Olden"
PTRDIST_DIR="$LLVM_TESTSUITE_DIR/MultiSource/Benchmarks/Ptrdist"
DATA_DIR="$MISC_DIR/data"

#
# Others
#
OS=`uname`

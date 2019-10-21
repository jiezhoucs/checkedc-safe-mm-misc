#!/bin/bash

LLVM_SRC=../llvm

rm -rf CMakeCache.txt

cmake -G "Unix Makefiles" \
      -DLLVM_TARGETS_TO_BUILD="X86" \
      -DCMAKE_BUILD_TYPE=Debug \
      -DLLVM_OPTIMIZED_TABLEGEN=ON \
      "$LLVM_SRC"

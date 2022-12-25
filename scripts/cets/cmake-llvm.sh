#!/bin/bash

#
# Set CC & CXX. Use clang & clang++ if exist.
#
if [[ `which clang` ]]; then
    CC="clang"
    CXX="clang++"
else
    CC="gcc"
    CXX="g++"
fi

LLVM_SRC=../llvm/llvm

rm -rf CMakeCache.txt

cmake -G "Unix Makefiles"                                   \
      -DCMAKE_C_COMPILER=$CC                                \
      -DCMAKE_CXX_COMPILER=$CXX                             \
      -DLLVM_TARGETS_TO_BUILD="X86"                         \
      -DCMAKE_BUILD_TYPE=Release                            \
      -DLLVM_ENABLE_ASSERTIONS=On                           \
      -DLLVM_OPTIMIZED_TABLEGEN=ON                          \
      -DLLVM_ENABLE_PROJECTS="clang;lld"                    \
      "$LLVM_SRC"

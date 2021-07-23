#!/bin/bash


rm -rf CMakeCache.txt

Build_Type="Debug"
if [[`uname` == "Linux" ]]; then
    Build_Type="Release"
fi

cmake -G "Unix Makefiles"                    \
      -DCMAKE_C_COMPILER="clang"             \
      -DCMAKE_CXX_COMPILER="clang++"         \
      -DLLVM_TARGETS_TO_BUILD="X86"          \
      -DCMAKE_BUILD_TYPE="$Build_Type"       \
      -DLLVM_ENABLE_ASSERTIONS=ON            \
      -DLLVM_OPTIMIZED_TABLEGEN=ON           \
      "$LLVM_SRC"

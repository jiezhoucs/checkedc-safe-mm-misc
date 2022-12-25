#!/bin/bash

# This script generates the Makefiles for the Checked C compiler.
# It is supposed to be invoked in the `root_dir/build` directory and it assumes
# that the Checked C src code is in `root_dir/llvm`.

rm -rf CMakeCache.txt

Build_Type="Debug"
if [[ `uname` == "Linux" ]]; then
    Build_Type="Release"
fi

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

cmake -G "Unix Makefiles"                    \
      -DCMAKE_C_COMPILER="$CC"               \
      -DCMAKE_CXX_COMPILER="$CXX"            \
      -DLLVM_TARGETS_TO_BUILD="X86"          \
      -DCMAKE_BUILD_TYPE="$Build_Type"       \
      -DLLVM_ENABLE_ASSERTIONS=ON            \
      -DLLVM_OPTIMIZED_TABLEGEN=ON           \
      "../llvm"

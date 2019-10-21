# Miscellaneous stuffs about the Checked C Temporal Memory Safety Project

This repository contains miscellaneous stuffs, small test programs,
development experience, handy scripts, etc., about the Checked C
temporal memory safety project.

## Build Suggestion
I suggest that you put all the related files about the project in one
big directory so that the relative paths in the scripts would work
for everyone.

### Build the Compiler

We have a `cmake` script `scripts/cmake-llvm.sh` to generate Makefiles for
building the project. If you're working on a \*nix system running on a
X86 processor, just copy the script to a build directory and run it.

Suggested building procedure:
```
1. Create a build directory in your Checked C root directory.
2. Copy the cmake-llvm.sh script to this build folder.
3. Run this script.
4. Run "make clang -jn" to build the compiler.
```

Here is Microsoft instructions on building the compiler: [Setting up your
machine and building
clang](https://github.com/Microsoft/checkedc-clang/blob/master/docs/checkedc/Setup-and-Build.md)

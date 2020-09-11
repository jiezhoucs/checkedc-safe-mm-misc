# Miscellaneous stuffs about the Checked C Temporal Memory Safety Project

This repository contains miscellaneous stuffs, small test programs,
development experience, handy scripts, etc., about the Checked C
temporal memory safety project.

## Experiment Suggestions
I suggest that you put all the related files about the project in one
big directory so that the relative paths in the scripts would work
for everyone.

### Directory Organization
Please organize your sub-directories in the Checked C project directory
in the following hierarchy:

```shell
checkedc                # root directory for the whole project
|-- build               # build directory of the LLVM compiler
|-- llvm                # LLVM src
|-- misc                # the safe-mm-misc repo
|-- tests
|   |-- test-suite      # LLVM test-suite: https://github.com/zhaishuang1/test-suite
|   |-- olden-build     # build directory of the Olden benchmark suite
|   |-- ptrdist-build   # build directory of the Ptrdist benchmark suite
```

### Build the Compiler

We have a `cmake` script `scripts/cmake-llvm.sh` to generate Makefiles for
building the project. If you're working on a \*nix system running on a
X86 processor, just copy the script to a build directory and run it.

Suggested building procedure:
```
cd checkedc-project-root-directory

git clone https://github.com/jzhou76/checkedc-safe-mm-misc.git misc

git clone https://github.com/jzhou76/checkedc-llvm llvm

cd llvm/tools

git clone https://github.com/jzhou76/checkedc-clang.git clang

cd ../projects/checkedc-wrapper

git clone https://github.com/jzhou76/checkedc.git

cd ../../.. ; mkdir build ; cd build

cp ../misc/scripts/cmake-llvm.sh ./

./cmake-llvm.sh

make clang -jn
```

Here is Microsoft instructions on building the compiler: [Setting up your
machine and building
clang](https://github.com/microsoft/checkedc-clang/blob/master/clang/docs/checkedc/Setup-and-Build.md)

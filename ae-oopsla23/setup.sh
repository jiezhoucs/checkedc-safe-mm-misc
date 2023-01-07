#!/usr/bin/env bash

#
# This script sets up the experiemental environment for the artifact evaluation
# of the OOPSLA'23 Checked C paper.
#

REQUIRED_DEPS=(
    "cmake"      # For building llvm
    "git"        # For downloading repos
    "git-lfs"    # For pulling down large input data files for evaluation
    "wget"       # For downloading the baseline llvm compiler
    "unzip"      # For unpackaging enwik9.
    "python2"    # llvm-lit uses `#/usr/bin/env python`, which could be python2
    "python3"    # For processing experimental data
    "pip3"       # For the use of numpy
    "ab"         # For evaluating thttpd. Available in "apache2-utils"
)

ROOT_DIR=`realpath .`
SCRIPTS_DIR="$ROOT_DIR/misc/scripts"
CETS_DIR="$ROOT_DIR/cets"
EVAL_SCRIPTS_DIR="$ROOT_DIR/misc/eval/scripts"

#
# Compiler and benchmark sources.
#
MISC_REPO="https://github.com/jzhou76/checkedc-safe-mm-misc.git"
LLVM_VANILLA_SRC="https://releases.llvm.org/8.0.0/llvm-8.0.0.src.tar.xz"
CLANG_VANILLA_SRC="https://releases.llvm.org/8.0.0/cfe-8.0.0.src.tar.xz"
CHECKEDC_LLVM_REPO="https://github.com/jzhou76/checkedc-llvm.git"
CHECKEDC_CLANG_REPO="https://github.com/jzhou76/checkedc-clang.git"
CHECKEDC_REPO="https://github.com/jzhou76/checkedc.git"
WSS_REPO="https://github.com/brendangregg/wss.git"
CETS_REPO="https://github.com/jzhou76/CETS-llvm8.git"
LLD_REPO="https://github.com/llvm-mirror/lld.git"
TEST_SUITE_REPO="https://github.com/jzhou76/test-suite.git"
ENWIK9_URL="http://mattmahoney.net/dc/enwik9.zip"

OOPSLA23_BRANCH="AE-OOPSLA23"
BUILD_FILES="build cets/build llvm-test-suite/ts-build* llvm-vanilla/build"
PARALLEL=`lscpu | grep "^CPU(s):" | cut -d ':' -f2` # | echo "$(cat -)" | bc`

#
# Check dependency
#
check_dep() {
    echo "Checking dependencies..."
    for dep in ${REQUIRED_DEPS[@]}; do
        echo -n "Looking for $dep..."
        if [[ ! `which $dep` ]]; then
            echo "not found. Please install $dep"
            exit
        else
            echo "found."
        fi
    done

    echo -n "Looking for numpy..."
    if [[ ! `pip3 list | grep numpy` ]]; then
        echo "not found. Please install the numpy package of Python3."
        exit
    else
        echo "found."
    fi
}

#
# Prepare for building compilers.
#
prepare_compiler() {
    # Check if dependency software exists.
    check_dep

    # Check if the misc directory exists.
    if [[ ! -d "misc" ]]; then
        echo "Pulling the checkedc-mm-safe-misc repo..."
        git clone "$MISC_REPO" misc
        # The misc repo has a wss submodule in it for memory overhead eval,
        # the the submodule was clone by @jzhou76 using ssh.
        # Just in case the user of this script does not set up ssh for git,
        # here we pull the wss repo separately.
        git clone "$WSS_REPO" "misc/eval/wss"
        cd "misc/eval/wss"; git checkout 8951296
        cd -

        # Replace the scripts with a soft link to the ones in the misc repo.
        rm -f setup.sh eval.sh print_results.sh
        ln -s misc/ae-oopsla23/setup.sh setup.sh
        ln -s misc/ae-oopsla23/eval.sh eval.sh
        ln -s misc/ae-oopsla23/print_results.sh print_results.sh
    fi

    # Prepare the vanilla LLVM 8.0.0 compiler.
    if [[ ! -d "llvm-vanilla" ]]; then
        mkdir -p llvm-vanilla; cd llvm-vanilla
        wget $LLVM_VANILLA_SRC
        tar -xf llvm-8.0.0.src.tar.xz; mv llvm-8.0.0.src llvm
        wget $CLANG_VANILLA_SRC
        tar -xf cfe-8.0.0.src.tar.xz; mv cfe-8.0.0.src llvm/tools/clang
        rm llvm-8.0.0.src.tar.xz cfe-8.0.0.src.tar.xz
        # Get lld
        git clone "$LLD_REPO" llvm/tools/lld; cd llvm/tools/lld; git checkout c51c3bc61
        cd -

        # Quick fix for potential compile errors.
        sed -i '7i #include <cstdint>' llvm/include/llvm/Demangle/MicrosoftDemangleNodes.h
        sed -i '7i #include <string>' llvm/include/llvm/Demangle/MicrosoftDemangleNodes.h
        cd "$ROOT_DIR"
    fi


    # Prepare the Checked C compiler.
    if [[ ! -d "llvm" ]]; then
        echo "Pulling the Checked C compiler repo..."
        git clone "$CHECKEDC_LLVM_REPO" llvm
        cd llvm; git checkout $OOPSLA23_BRANCH; cd -
        git clone "$CHECKEDC_CLANG_REPO" llvm/tools/clang
        cd llvm/tools/clang; git checkout $OOPSLA23_BRANCH; cd -
        git clone "$CHECKEDC_REPO" llvm/projects/checkedc-wrapper/checkedc
        cd llvm/projects/checkedc-wrapper/checkedc; git checkout $OOPSLA23_BRANCH; cd -
        # Get lld
        cp -r "$ROOT_DIR/llvm-vanilla/llvm/tools/lld" llvm/tools/lld
        cd "$ROOT_DIR"
    fi

    # Prepare the CETS compiler.
    if [[ ! -d "cets" ]]; then
        echo "Pulling the CETS compiler repo..."
        mkdir cets
        git clone "$CETS_REPO" cets/llvm
        cd cets/llvm; git checkout $OOPSLA23_BRANCH
    fi

    cd $ROOT_DIR
}

#
# Print out usage and exit
#
usage() {
    # TODO
    echo "Help"
}

build_compiler() {
    make clang -j$PARALLEL
    make lld -j$PARALLEL
    make llvm-ar
    make llvm-size
}

#
# Build the Checked C compiler.
#
build_baseline() {
    cd $ROOT_DIR
    echo "Building the baseline LLVM compiler"
    if [[ -f "llvm-vanilla/build/bin/clang" ]]; then
        cd llvm-vanilla/build
        build_compiler
    else
        mkdir -p llvm-vanilla/build; cd llvm-vanilla/build
        cp "$SCRIPTS_DIR/cmake-llvm.sh" ./
        ./cmake-llvm.sh
        build_compiler
    fi
}

#
# Build the Checked C compiler.
#
build_checkedc() {
    cd $ROOT_DIR
    echo "Building the Checked C compiler"
    if [[ -f "build/bin/clang" ]]; then
        cd build
        build_compiler
    else
        mkdir -p build; cd build
        cp "$SCRIPTS_DIR/cmake-llvm.sh" ./
        ./cmake-llvm.sh
        build_compiler
        make llvm-ranlib
    fi

    # Compile libsafemm
    cd "$ROOT_DIR/misc/lib"
    if [[ ! -f "libsafemm.a" ]]; then
        make
    fi
    if [[ ! -f "libsafemm_lto.a" ]]; then
        # Compile libsafemm_lto
        make lto
    fi

}

#
# Build the CETS compiler.
#
build_cets() {
    cd $ROOT_DIR
    echo "Building the CETS compiler"
    if [[ -f "cets/build/bin/clang" ]]; then
        cd cets/build
        build_compiler
    else
        mkdir -p cets/build; cd cets/build
        cp "$SCRIPTS_DIR/cets/cmake-llvm.sh" ./
        ./cmake-llvm.sh
        build_compiler
    fi

    # Build the CETS runtime lib.
    cd "$CETS_DIR/llvm/libsoftboundcets"
    make lto
}

build_all_compilers() {
    build_baseline
    build_checkedc
    build_cets
}

#------------------------------------------------------------------------------#

#
# Prepare the benchmarks, including setting up llvm test-suite and downloading
# data inputs.
#
prepare_benchmark() {
    cd "$ROOT_DIR"
    mkdir -p llvm-test-suite; cd llvm-test-suite
    # Download our modified llvm test-suite repo. The default branch contains
    # the Checked C version of Olden benchmarks.
    if [[ ! -d test-suite ]]; then
        git clone "$TEST_SUITE_REPO"
        cd "$SCRIPTS_DIR"
        ./cmake-ts.sh lto
        cd -
    fi

    if [[ ! -d test-suite-baseline ]]; then
        # Make a copy and checkout the baseline test-suite code.
        cp -r test-suite test-suite-baseline
        cd test-suite-baseline
        git checkout baseline

        # Generate the test-suite build files for baseline llvm.
        cd "$SCRIPTS_DIR"
        ./cmake-ts-baseline.sh lto

        # Generate the test-suite build files for CETS
        cd "$SCRIPTS_DIR/cets"
        ./cmake-ts.sh lto
    fi

    # Download enwik9 for lzfse. We download this data input separately because
    # it is very large and it is better to avoid store it directly in github.
    cd "$ROOT_DIR/misc/eval/lzfse_dataset"
    if [[ ! -f enwik9 ]]; then
        wget http://mattmahoney.net/dc/enwik9.zip
        unzip enwik9.zip
    fi
}

#------------------------------------------------------------------------------#

#
# Check if all required artifacts have been installed successfully.
#
post_build_check() {
    echo ""
    echo "Post-build checking..."

    compilers=(
        "baseline"
        "Checked-C"
        "CETS"
    )

    tools=(
        "clang"
        "lld"
        "llvm-ar"
        "llvm-lit"
    )

    cd "$ROOT_DIR"
    # Checking compilers and related tools.
    for compiler in ${compilers[@]}; do
        if [[ $compiler == "baseline" ]]; then
            cd "$ROOT_DIR/llvm-vanilla"
        elif [[ $compiler == "CETS" ]]; then
            cd "$ROOT_DIR/cets"
        else
            cd "$ROOT_DIR"
        fi

        for tool in ${tools[@]}; do
            echo -n "Checking $compiler $tool..."
            if [[ -f "build/bin/$tool" ]]; then
                echo "found."
            else
                echo "not found. Something wrong happened building $tool."
            fi
        done
    done

    # Checking the Checked C's and CETS' runtime libs
    cd $ROOT_DIR
    echo -n "Checking Checked C's runtime lib..."
    if [[ -f "misc/lib/libsafemm.a" &&  -f "misc/lib/libsafemm_lto.a" ]]; then
        echo "found."
    else
        echo "not found. Something wrong happened building Checked C's runtime library."
    fi

    echo -n "Checking CETS' runtime lib..."
    if [[ -f "cets/llvm/libsoftboundcets/lto/libsoftboundcets_rt.a" ]]; then
        echo "found."
    else
        echo "not found. Something wrong happened building CETS' runtime library."
    fi

    # Check if wss and enwik9 have been downloaded
    cd $ROOT_DIR
    if [[ ! -f "misc/eval/wss/wss.pl" ]]; then
        echo "wss was missing!"
    fi

    if [[ ! -f "misc/eval/lzfse_dataset/enwik9" ]]; then
        echo "enwik9 was missing!"
    fi
}

#
# Entrance of this script
#
if [[ $# > 1 ]]; then
    echo "Unknown arguments"
        usage
    exit
elif [[ $# == 0 ]]; then
    prepare_compiler

    build_all_compilers

    prepare_benchmark
else
    if [[ $1 == "checkedc" ]]; then
        build_checkedc
    elif [[ $1 == "cets" ]]; then
        build_cets
    elif [[ $1 == "baseline" ]]; then
        build_baseline
    elif [[ $1 == "clean-all" ]]; then
        rm -rf "$BUILD_FILES"
    elif [[ $1 == "-h" ]]; then
        usage
    else
        echo "Unknown arguments"
        usage
        exit
    fi
fi

post_build_check

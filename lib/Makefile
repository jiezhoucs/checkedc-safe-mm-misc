#
# Compiler and other tool settings
#
ROOT_DIR := $(realpath $(dir $(lastword $(MAKEFILE_LIST))))/../..
MISC_DIR := $(ROOT_DIR)/misc
LLVM_DIR := $(ROOT_DIR)/build/bin
CC 		 := $(LLVM_DIR)/clang
OPT 	 := -O3
CFLAGS   := $(OPT) -I$(MISC_DIR)/include -Wall -mrdrnd -fPIC
# Have to use a native clang++. The one in the Checked C directory somehow breaks.
CXX 	 := clang++
CXXFLAGS := $(OPT) -fPIC 
AR 		 := $(LLVM_DIR)/llvm-ar
RANLIB   := $(LLVM_DIR)/llvm-ranlib

#
# Source code
#
LIB_SRC   := safe_mm_checked.c mm_libc.c mm_common.c
PORT_SRC  := porting_helper.cpp
DEBUG_SRC := debug.c

#
# Buld targets
#
LIB_SAFEMM     	   := libsafemm
LIB_SAFEMM_LTO     := $(LIB_SAFEMM)_lto
LIB_SAFEMM_PORTING := libsafemm_porting
LIB_PORTING  	   := libporting
LIB_DEBUG		   := libdebug

#
# Function: Compile C/C++ source and archive objective files into a static library.
#
# Parameters:
#   1: Compiler (clang or clang++)
#	2: List of Source files
#	3: C/CXX flags
#	4: Target name of the static library
# 
define build_target
	@echo "Bulding$(4) ..."
	$(1) $(2) -c $(3)
	$(AR) -rc $(4).a *.o
	$(RANLIB) $(4).a
	@rm -f *.o
	@echo "Finished building$(4)\n"
endef

all: $(LIB_SAFEMM) $(LIB_PORTING) $(LIB_SAFEMM_PORTING)

#
# Compile the libsafemm to a static library.
#
$(LIB_SAFEMM): $(LIB_SRC)
	$(call build_target, $(CC), $^, $(CFLAGS), $@)

$(LIB_SAFEMM_LTO): $(LIB_SRC)
	$(call build_target, $(CC), $^, $(CFLAGS) -flto, $@)

#
# Compile the CPP lib for porting assistance.
#
$(LIB_PORTING): $(PORT_SRC)
	$(call build_target, $(CXX), $^, $(CXXFLAGS), $@)

#
# libsafemm with "PORTING" defined.
#
$(LIB_SAFEMM_PORTING): $(LIB_SRC)
	$(call build_target, $(CC), $^, $(CFLAGS) -DPORTING, $@)

#
# Compile libsafemm and libporting for debugging.
#
debug: CFLAGS += -O0 -g -DMM_DEBUG
debug: CXXFLAGS += -O0 -g -DMM_DEBUG
debug: all;

#
# Compile libdebug
#
libdebug: debug.c
	$(call build_target, $(CC), $^, $(CFLAGS), $@)

#
# Compile libsafemm to LLVM IR code for debugging.
#
ll: $(LIB_SRC)
	$(CC) $(CFLAGS) -S -emit-llvm $^

clean:
	rm -rf *.o *.so *.a *.s *.ll
#
# LLVM build directory
#
LLVM_DIR = ../../build/bin
CHECKEDC_MISC = ../lib

OPT=-O3
CFLAGS = $(OPT) -I../include

#
# Checked-C Clang
#
CC = $(LLVM_DIR)/clang $(CFLAGS)

LIB_SRC = safe_mm_checked.c

so: $(LIB_SRC)
	$(CC) -shared -fPIC -o lib-safemm.so $^

ll: $(LIB_SRC)
	$(CC) -S -emit-llvm $^

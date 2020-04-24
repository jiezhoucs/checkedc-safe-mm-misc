#
# LLVM build directory
LLVM_DIR = ../../build/bin

OPT=-O3
CFLAGS = $(OPT) -I../include

#
# Checked-C Clang
CC = $(LLVM_DIR)/clang $(CFLAGS)

LIB_SRC = safe_mm_checked.c
LIB_SAFEMM = libsafemm

.PHONY: libsafemm clean

# Compile both a static library and a dynamic library.
libsafemm: a so

#
# Compile the lib to a static library.
a: $(LIB_SRC)
	$(CC) -c $(LIB_SRC) -o $(LIB_SAFEMM).o
	ar -rc $(LIB_SAFEMM).a $(LIB_SAFEMM).o
	@ranlib $(LIB_SAFEMM).a
	@rm -f $(LIB_SAFEMM).o

#
# Compile the lib to a shared libray. Note that on MacOS .dylib is the
# native format for shared library, and .so is for compatibility.
so: $(LIB_SRC)
	$(CC) -shared -fPIC -o $(LIB_SAFEMM).so $^

#
# Compile to LLVM IR code for debugging purpose.
ll: $(LIB_SRC)
	$(CC) -S -emit-llvm $^

clean:
	rm -rf *.o *.so *.a

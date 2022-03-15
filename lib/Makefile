#
# LLVM build directory
LLVM_DIR = ../../build/bin

OPT=-O3
CFLAGS = $(OPT) -I../include -Wall -mrdrnd -fPIC

#
# Checked-C Clang
CC = $(LLVM_DIR)/clang $(CFLAGS)
AR = $(LLVM_DIR)/llvm-ar
RANLIB = $(LLVM_DIR)/llvm-ranlib

LIB_SRC = safe_mm_checked.c
LIB_SAFEMM = libsafemm
LIB_SAFEMM_LTO = $(LIB_SAFEMM)_lto

.PHONY: libsafemm clean

# Compile both a static library and a dynamic library.
libsafemm: a lto

#
# Compile the lib to a static library.
a: $(LIB_SRC)
	rm -f $(LIB_SAFEMM).a $(LIB_SAFEMM).so
	$(CC) -c $(LIB_SRC) -o $(LIB_SAFEMM).o
	$(AR) -rc $(LIB_SAFEMM).a $(LIB_SAFEMM).o
	$(RANLIB) $(LIB_SAFEMM).a
	@rm -f $(LIB_SAFEMM).o

lto: $(LIB_SRC)
	rm -rf $(LIB_SAFEMM_LTO).a $(LIB_SAFEMM_LTO).so
	$(CC) -flto -c $(LIB_SRC) -o $(LIB_SAFEMM_LTO).o
	$(AR) -rc $(LIB_SAFEMM_LTO).a $(LIB_SAFEMM_LTO).o
	$(RANLIB) $(LIB_SAFEMM_LTO).a
	@rm -f *.o


#
# Compile the CPP lib for porting assistance.
#
# Have to use a native clang++; the one in the Checked C directory breaks.
CXX := clang++
CXXFLAGS := -fPIC -O3
LIB_PORTING := libporting
port: porting_helper.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $(LIB_PORTING).o
	llvm-ar -rc $(LIB_PORTING).a $(LIB_PORTING).o
	llvm-ranlib $(LIB_PORTING).a
	@rm -f *.o

#
# Compile the lib to a shared libray. Note that on MacOS .dylib is the
# native format for shared library, and .so is for compatibility.
so: $(LIB_SRC)
	$(CC) -shared -fPIC -o $(LIB_SAFEMM).so $^

#
# Compile to LLVM IR code for debugging purpose.
ll: $(LIB_SRC)
	$(CC) -S -emit-llvm $^

#
# Compile libdebug
debug: debug.c
	$(CC) -c $^ -o $@.o
	ar -rc libdebug.a $@.o
	@ranlib libdebug.a
	@rm -f $@.o

debugso: debug.c
	$(CC) -shared -fPIC -o libdebug.so $^

clean:
	rm -rf *.o *.so *.a *.s *.ll

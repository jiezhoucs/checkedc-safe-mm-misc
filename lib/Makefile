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

LIB_SRC = safe_mm_checked.c mm_libc.c mm_common.c
LIB_SAFEMM = libsafemm
LIB_SAFEMM_LTO = $(LIB_SAFEMM)_lto
LIB_SAFEMM_PORTING = libsafemm_porting
PORT_SRC = porting_helper.cpp

.PHONY: libsafemm clean

# Compile both a static library and a dynamic library.
all: a port # lto

#
# Compile the lib to a static library.
a: $(LIB_SRC)
	rm -f $(LIB_SAFEMM).a $(LIB_SAFEMM).so *.o
	$(CC) -c $(LIB_SRC)
	$(AR) -rc $(LIB_SAFEMM).a *.o
	$(RANLIB) $(LIB_SAFEMM).a
	@rm -f *.o

lto: $(LIB_SRC)
	rm -rf $(LIB_SAFEMM_LTO).a $(LIB_SAFEMM_LTO).so
	$(CC) -flto -c $(LIB_SRC)
	$(AR) -rc $(LIB_SAFEMM_LTO).a *.o
	$(RANLIB) $(LIB_SAFEMM_LTO).a
	@rm -f *.o


#
# Compile the CPP lib for porting assistance.
#
# Have to use a native clang++; the one in the Checked C directory breaks.
CXX := clang++
CXXFLAGS := -fPIC $(OPT)
LIB_PORTING := libporting
port: $(PORT_SRC)
	$(CXX) $(CXXFLAGS) -c $^ -o $(LIB_PORTING).o
	$(AR) -rc $(LIB_PORTING).a $(LIB_PORTING).o
	$(RANLIB) $(LIB_PORTING).a
	@rm -f *.o

#
# Compile libsafemm and libporting for debugging.
#
debug: CFLAGS += -O0 -g -DMM_DEBUG
debug: CXXFLAGS += -O0 -g -DMM_DEBUG
debug: a port;

#
# libsafemm with "PORTING" defined.
#
lib_porting: $(LIB_SRC)
	rm -f *.o
	$(CC) -DPORTING -Wno-implicit-function-declaration -c $(LIB_SRC)
	$(AR) -rc $(LIB_SAFEMM_PORTING).a *.o
	$(RANLIB) $(LIB_SAFEMM_PORTING).a
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
debuglib: debug.c
	$(CC) -c $^ -o $@.o
	$(AR) -rc libdebug.a $@.o
	@$(RANLIB) libdebug.a
	@rm -f $@.o

debuglib_so: debug.c
	$(CC) -shared -fPIC -o libdebug.so $^

clean:
	rm -rf *.o *.so *.a *.s *.ll

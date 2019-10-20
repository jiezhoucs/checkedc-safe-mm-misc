#
# LLVM build directory
#
LLVM_DIR = ../../../build/bin

OPT=-O0
CFLAGS = $(OPT) -I../include

#
# Checked-C Clang
#
CC = $(LLVM_DIR)/clang $(CFLAGS)

SRC = hello.c

hello: $(SRC)
	$(CC) $^ -o hello
	objdump -d hello > hello.s

ll: $(SRC)
	$(CC) -S -emit-llvm $^

clean:
	rm -f *.o *.s *.ll *.tmp hello

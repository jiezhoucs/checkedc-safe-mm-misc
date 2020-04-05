#
# LLVM build directory
#
LLVM_DIR = ../../build/bin

OPT=-O0
CFLAGS = $(OPT) -I../include

#
# Checked-C Clang
#
CC = $(LLVM_DIR)/clang $(CFLAGS)

SRC = dereference.c assign.c
OBJ = $(SRC:%.c=%.o)
ASM = $(SRC:%.c=%.s)
LL = $(SRC:%.l=$.ll)
BIN = $(SRC:%.c=%)

HELLO = hello.c

#
# Compile hello.c to test the currently developed functionality.
#
hello: $(HELLO)
	$(CC) $^ -o hello
	objdump -d hello > hello.s

ll: $(HELLO)
	$(CC) -S -emit-llvm $^

o1: $(HELLO)
	$(CC) -S -O1 -emit-llvm $^

o2: $(HELLO)
	$(CC) -S -O2 -emit-llvm $^


#
# Regression test
#
rt: $(SRC)
	for src in $(BIN) ; do \
		$(CC) $$src.c -o $$src ; \
	done

clean: $(SRC)
	rm -f $(BIN) *.s *.o *.tmp *.ll hello a.out

#
# LLVM build directory
#
LLVM_DIR = ../../build/bin
CHECKEDC_MISC = ../lib
UNAME_S := $(shell uname -s)

OPT=-O3
CFLAGS = $(OPT) -I../include
LDFLAGS = -L../lib -lsafemm -ldebug -rdynamic
ifeq ($(UNAME_S), Darwin)
	LDFLAGS += -Xlinker -syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
else ifeq ($(UNAME_S), Linux)
	LDFLAGS += -Wl,-rpath,../lib
endif
PRINT_PASSES = -mllvm -opt-bisect-limit=-1

#
# Checked-C Clang
#
CC = $(LLVM_DIR)/clang $(CFLAGS)

SRC = basic.c assign.c dereference.c func.c cast.c array.c addressof.c checkable.c
LIB = $(CHECKEDC_MISC)/lib-safemm.c
OBJ = $(SRC:%.c=%.o)
ASM = $(SRC:%.c=%.s)
LL = $(SRC:%.l=$.ll)
BIN = $(SRC:%.c=%)

HELLO = hello.c

#
# Compile hello.c to test the currently developed functionality.
#
hello: $(HELLO)
	$(CC) $^ $(LDFLAGS) -o hello
	objdump -d hello > hello.s

debug: $(HELLO)
	$(CC) -g -O0 $^ $(LDFLAGS) -o hello

ll: $(HELLO)
	$(CC) -S -emit-llvm $^

o0: $(HELLO)
	$(CC) -S -O0 -emit-llvm $^

o1: $(HELLO)
	$(CC) -S -O1 -emit-llvm $^

o2: $(HELLO)
	$(CC) -S -O2 -emit-llvm $^

o3: $(HELLO)
	$(CC) -S -O3 -emit-llvm $^


#
# Regression test
#
rt: $(SRC)
	for src in $(BIN) ; do \
		$(CC) $$src.c $(LDFLAGS) -o $$src ; \
	done

basic: basic.c
	$(CC) $(LDFLAGS) $^ -o basic

assign: assign.c
	$(CC) $(LDFLAGS) $^ -o assign

deref: dereference.c
	$(CC) $(LDFLAGS) $^ -o dereference

func: func.c
	$(CC) $(LDFLAGS) $^ -o func

cast: cast.c
	$(CC) $(LDFLAGS) $^ -o cast

array: array.c
	$(CC) $(LDFLAGS) $^ -o array

addrof: addressof.c
	$(CC) $(LDFLAGS) $^ -o addressof

chk: checkable.c
	$(CC) $(LDFLAGS) $^ -o checkable

opt: opt.c
	$(CC) -S -O1 -emit-llvm $^

clean: $(SRC)
	rm -rf $(BIN) *.s *.o *.bc *.i *.tmp *.ll *.stats hello a.out *.dSYM

CC = ../../../../llvm-vanilla/build/bin/clang
CFLAGS = -O3 -Wall -Wextra -pedantic-errors -Wno-unused-parameter

SYSTEM = $(shell uname -s)

ifeq ($(SYSTEM), Darwin)
	LDFLAGS += -Xlinker -syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
endif

TEST_SRC = tests.c parson.c
EVAL_SRC = eval.c parson.c
TET_OBJ = $(SRC:%.c=%.o)

all: test eval

.PHONY: test lto eval clean
test: $(TEST_SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ tests.c parson.c
	./$@

lto: $(EVAL_SRC)
	$(CC) -v $(CFLAGS) -flto -fuse-ld=lld -Wl,-flto -o test $^
	./test

eval: $(EVAL_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

eval_mem: $(EVAL_SRC)
	$(CC) -DMEM_OVERHEAD $(CFLAGS) -o eval $^ $(LDFLAGS)

clean:
	rm -f eval test *.o


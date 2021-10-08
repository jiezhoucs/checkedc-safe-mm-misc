ROOT_DIR := $(shell realpath ../../../../)
MISC_DIR := $(ROOT_DIR)/misc

CC := $(ROOT_DIR)/build/bin/clang
CFLAGS := -O3 -Wall
CFLAGS += -I$(MISC_DIR)/include
LDFLAGS := -L$(MISC_DIR)/lib -lsafemm -ldebug

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
	$(CC) $(CFLAGS)  -o $@ $^ $(LDFLAGS)
	./$@

lto: $(TEST_SRC)
	$(CC) $(CFLAGS) -flto -c $^
	$(CC) -fuse-ld=lld  $(MISC_DIR)/lib/libsafemm_lto.a -o test $(OBJ)
	./test

eval: $(EVAL_SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

eval_mem: $(EVAL_SRC)
	$(CC) -DMEM_OVERHEAD $(CFLAGS) -o eval $^ $(LDFLAGS)

clean:
	rm -f test eval *.o


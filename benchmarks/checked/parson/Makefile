ROOT_DIR := $(shell realpath ../../../../)
MISC_DIR := $(ROOT_DIR)/misc

CC := $(ROOT_DIR)/build/bin/clang
CFLAGS := -O3 -Wall  -std=c89 # -pedantic-errors
CFLAGS += -I$(MISC_DIR)/include
LDFLAGS := -L$(MISC_DIR)/lib -lsafemm -ldebug

SYSTEM = $(shell uname -s)

ifeq ($(SYSTEM), Darwin)
	LDFLAGS += -Xlinker -syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
endif

SRC = tests.c parson.c

all: test

.PHONY: test
test: $(SRC)
	$(CC) $(CFLAGS)  -o $@ $^ $(LDFLAGS)
	./$@

lto: $(SRC)
	$(CC) $(CFLAGS) -flto -fuse-ld=lld $(MISC_DIR)/lib/libsafemm_lto.a -o test $^
	./test

clean:
	rm -f test *.o


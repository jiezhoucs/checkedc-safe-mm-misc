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

all: test

.PHONY: test
test: tests.c parson.c
	$(CC) $(CFLAGS)  -o $@ tests.c parson.c $(LDFLAGS)
	./$@

clean:
	rm -f test *.o


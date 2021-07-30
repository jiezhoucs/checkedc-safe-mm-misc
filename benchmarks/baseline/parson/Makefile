CC = ../../../../llvm-vanilla/build/bin/clang
CFLAGS = -O3 -Wall -Wextra -std=c89 -pedantic-errors

SYSTEM = $(shell uname -s)

ifeq ($(SYSTEM), Darwin)
	LDFLAGS += -Xlinker -syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk
endif

all: test testcpp

.PHONY: test testcpp
test: tests.c parson.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ tests.c parson.c
	./$@

clean:
	rm -f test *.o


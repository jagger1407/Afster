CC = gcc
SRC = $(wildcard *.c)

all: debug release

debug: CFLAGS = -g -O0
debug: $(patsubst %.c,../build/debug/%,$(SRC))

release: CFLAGS = -O2
release: $(patsubst %.c,../build/release/%,$(SRC))

../build/debug/%: %.c
	$(CC) $(CFLAGS) $< -o $@ -L../build/debug -lAfster -Wl,-rpath,'$$ORIGIN'

../build/release/%: %.c
	$(CC) $(CFLAGS) $< -o $@ -L../build/release -lAfster -Wl,-rpath,'$$ORIGIN'


clean:
	find ../build/debug -type f -not -name "libAfster.so" -delete
	find ../build/release -type f -not -name "libAfster.so" -delete



.PHONY: all debug release clean

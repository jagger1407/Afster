CC = gcc
SRC = afs.c afl.c

all: debug release

debug: CFLAGS = -g -O0
debug: build/debug/libAfster.so

release: CFLAGS = -O2
release: build/release/libAfster.so

build/debug/libAfster.so: $(SRC) | build/debug
	$(CC) $(CFLAGS) -fPIC -shared $(SRC) -o $@

build/release/libAfster.so: $(SRC) | build/release
	$(CC) $(CFLAGS) -fPIC -shared $(SRC) -o $@

build/debug build/release:
	mkdir -p $@

clean:
	rm -rf build/*

.PHONY: all debug release clean

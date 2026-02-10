CC = gcc
SRC = afs.c afl.c
UNAME = $(shell uname -s)

ifeq ($(UNAME),Linux)
	TARGET = libAfster.so
	BINDIR = build/linux
endif
ifeq ($(OS),Windows_NT)
	TARGET = Afster.dll
	BINDIR = build/win
	IMPLIB = -Wl,--out-implib,libAfster.a -Wl,--export-all-symbol Wl,--enable-auto-import
endif

all: debug release

debug: CFLAGS = -g -O0 -DBUILDING
debug: $(BINDIR)/debug/$(TARGET)

release: CFLAGS = -O2 -DBUILDING
release: $(BINDIR)/release/$(TARGET)

$(BINDIR)/debug/$(TARGET): $(SRC) | $(BINDIR)/debug
	$(CC) $(CFLAGS) -fPIC -shared $(SRC) -o $@ $(IMPLIB)

$(BINDIR)/release/$(TARGET): $(SRC) | $(BINDIR)/release
	$(CC) $(CFLAGS) -fPIC -shared $(SRC) -o $@ $(IMPLIB)

$(BINDIR)/debug $(BINDIR)/release:
	mkdir -p $@

examples: examples_debug examples_release


examples_debug: $(BINDIR)/debug/$(TARGET)
	$(MAKE) -C examples debug

examples_release: $(BINDIR)/release/$(TARGET)
	$(MAKE) -C examples release

clean:
	rm -rf build

.PHONY: all debug release examples examples_debug examples_release clean

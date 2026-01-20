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

clean:
	find build -name $(TARGET) -delete

.PHONY: all debug release clean

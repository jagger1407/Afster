CC = gcc
SRC = $(wildcard *.c)
BINDIR = ../build/linux

all: debug release

debug: CFLAGS = -g -O0
debug: $(patsubst %.c,$(BINDIR)/debug/%,$(SRC))

release: CFLAGS = -O2
release: $(patsubst %.c,$(BINDIR)/release/%,$(SRC))

$(BINDIR)/debug/%: %.c | $(BINDIR)/debug/libAfster.so
	$(CC) $(CFLAGS) $< -o $@ -L$(BINDIR)/debug -lAfster -Wl,-rpath,'$$ORIGIN'

$(BINDIR)/release/%: %.c | $(BINDIR)/release/libAfster.so
	$(CC) $(CFLAGS) $< -o $@ -L$(BINDIR)/release -lAfster -Wl,-rpath,'$$ORIGIN'

$(BINDIR)/debug/libAfster.so:
	cd .. && make debug && pwd

$(BINDIR)/release/libAfster.so:
	cd .. && make release && pwd

clean:
	find $(BINDIR)/debug -type f -not -name "libAfster.so" -delete
	find $(BINDIR)/release -type f -not -name "libAfster.so" -delete



.PHONY: all debug release clean

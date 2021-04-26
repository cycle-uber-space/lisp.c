
.POSIX:
.SUFFIXES:
.PHONY: all clean

CFLAGS = -std=c11 -Wall -Wextra -Wno-unused-parameter -O2
LDFLAGS = -Wall -Wextra -O2

all: lisp

clean:
	rm -f lisp.o
	rm -f lisp

lisp: lisp.o
	cc $(LDFLAGS) -o $@ $<

lisp.o: lisp.c
	cc -c $(CFLAGS) -o $@ $<


.POSIX:
.SUFFIXES:
.PHONY: all clean

all: lisp

clean:
	rm -f lisp

lisp: lisp.c
	cc -std=c11 -Wall -Wextra -Wno-unused-parameter -O2 -o $@ $<

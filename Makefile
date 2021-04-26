
.POSIX:
.SUFFIXES:
.PHONY: all clean

CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wno-unused-parameter -O2
LDFLAGS = -Wall -Wextra -O2

all: lisp

clean:
	rm -f lisp.o
	rm -f lisp

lisp: lisp.o
	$(CC) $(LDFLAGS) -o $@ $<

lisp.o: lisp.c
	$(CC) -c $(CFLAGS) -o $@ $<

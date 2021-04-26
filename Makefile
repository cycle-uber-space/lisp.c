
.POSIX:
.SUFFIXES:
.PHONY: all clean

CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wno-unused-parameter -O2
LDFLAGS = -Wall -Wextra -O2

OBJ = main.o global.o

all: lisp

clean:
	rm -f $(OBJ)
	rm -f lisp

lisp: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c lisp.h
	$(CC) -c $(CFLAGS) -o $@ $<


.POSIX:
.SUFFIXES:
.PHONY: all clean

CC = cc
CFLAGS = -std=c11 -Wall -Wextra -Wno-unused-parameter -Werror-implicit-function-declaration
LDFLAGS = -Wall -Wextra

# debug
#CFLAGS += -g -O0
#LDFLAGS += -O0

# release
CFLAGS += -O3
LDFLAGS += -s -O3

OBJ = test.o error.o expr.o symbol.o cons.o gensym.o string.o stream.o special.o builtin.o reader.o printer.o util.o env.o core.o eval.o system.o global.o main.o

all: lisp

clean:
	rm -f $(OBJ)
	rm -f lisp

lisp: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c lisp.h Makefile
	$(CC) -c $(CFLAGS) -o $@ $<

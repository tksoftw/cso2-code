.PHONY: all clean

CC = gcc
CFLAGS = -Wall -g -O3
LDFLAGS = -L.
LDLIBS = -lmlpt

all: tlb.out

tlb.out: tlb.o
	$(CC) tlb.o -o tlb.out $(LDFLAGS) $(LDLIBS)

tlb.o: tlb.c tlb.h mlpt.h config.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f tlb.o tlb.out

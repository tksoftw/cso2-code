.PHONY: all clean

CC = gcc
CFLAGS = -Wall -g

all: libmlpt.a

libmlpt.a: pagetable.o
	ar rcs $@ $^

pagetable.o: pagetable.c mlpt.h config.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f pagetable.o libmlpt.a
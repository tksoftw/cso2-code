.PHONY: all clean

CC = gcc
CFLAGS = -D_XOPEN_SOURCE=700 -Og -g -std=c11 -Wall -pedantic
LDFLAGS = -fsanitize=address -g

all: split

split: main.o split.o
	$(CC) $(CFLAGS) main.o split.o -o split $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o split
CC = gcc
CFLAGS =  -D_XOPEN_SOURCE=700 -O1 -g -std=c11 -Wall -pedantic
LDFLAGS = 

all: gettimings

gettimings: timing.c
	$(CC) $(CFLAGS) timing.c -o gettimings $(LDFLAGS)

clean:
	rm -f gettimings

.PHONY: all, clean

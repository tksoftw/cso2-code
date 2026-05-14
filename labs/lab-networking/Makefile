CC = clang
CFLAGS = -g -Og -Wall
LDFLAGS = -L. -lnetsim

.PHONY: all clean

all: netlab

netlab: netlab.o
	$(CC) netlab.o -o netlab $(LDFLAGS) 

%.o : %.c netsim.h
	$(CC) -c $(CFLAGS) $< -o $@


clean:
	rm -f *.o netlab

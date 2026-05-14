CC=gcc
CFLAGS=-Wall -pedantic -Og -ggdb

all: lab

lab: attack.o main.o libsecurelab.a
	$(CC) $(CFLAGS) $(LDFLAGS) -o lab $^ -ldl -lpthread

%.o: %.c lab.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f attack.o main.o

.PHONY: all clean

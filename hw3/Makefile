CC = gcc
CFLAGS = -Wall -fsanitize=address -ftrivial-auto-var-init=pattern -Og -g
LDFLAGS = 
.PHONY: all clean

all: fork

fork: fork_run.c
	$(CC) $(CFLAGS) fork_run.c -o fork

clean:
	rm -f fork
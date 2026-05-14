# Default build
CC = gcc
CFLAGS ?= -Wall -Wextra -g -O2
LDFLAGS ?= -pthread


OBJS = pool.o pool-test.o

.PHONY: all clean test tsan

all: pool-test

pool-test: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)

pool.o: pool.c pool.h
pool-test.o: pool-test.c pool.h

clean:
	rm -f pool-test $(OBJS)

test: pool-test
	./pool-test

# ThreadSanitizer (recommended in assignment instructions)
# setarch -R disables ASLR to work around a TSan/kernel shadow-memory
# incompatibility on recent Linux kernels (e.g. WSL2).
# We `clean` both before and after so tsan-instrumented .o files don't
# poison a subsequent plain `make`.
tsan:
	$(MAKE) clean
	$(MAKE) pool-test CFLAGS="$(CFLAGS) -fsanitize=thread -g" LDFLAGS="$(LDFLAGS) -fsanitize=thread"
	setarch "$$(uname -m)" -R ./pool-test; status=$$?; $(MAKE) clean; exit $$status

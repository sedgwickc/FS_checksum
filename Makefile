CC=gcc

CFLAGS=-Wall -std=c99 -DMEMWATCH -DMW_STDIO -c -g

all: FSchecksum

FSchecksum: FSchecksum.o memwatch.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $^ 

test:

clean:
	rm *.o FSchecksum

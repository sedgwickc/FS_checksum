CC=gcc

CFLAGS=-Wall -std=c99 -DMEMWATCH -DMW_STDIO -c -g -D_POSIX_SOURCE 

all: FSchecksum

FSchecksum: FSchecksum.o logging.o fschksum_gen.o memwatch.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $^ 

test:

clean:
	rm *.o FSchecksum

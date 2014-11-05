CC=gcc

CFLAGS=-Wall -std=c99 -DMEMWATCH -DMW_STDIO -DMW_PTHREADS -c -D_POSIX_SOURCE

CFLAGS_DEBUG=-Wall -std=c99 -DMEMWATCH -DMW_STDIO -DMW_PTHREADS -c -g -D_POSIX_SOURCE -DDEBUG 

CFLAGS_LINK=-lcrypto -pthread

all: FSchecksum

debug: FSchecksum_debug

FSchecksum_debug: FSchecksum_db.o logging_db.o bound_buff_db.o memwatch_db.o
	$(CC) $(CFLAGS_LINK) $^ -o $@

FSchecksum: FSchecksum.o logging.o bound_buff.o memwatch.o
	$(CC) $(CFLAGS_LINK) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $^ 

%_db.o: %.c
	$(CC) $(CFLAGS_DEBUG) -o $@ $^ 

test:

clean:
	rm *.o FSchecksum FSchecksum_debug

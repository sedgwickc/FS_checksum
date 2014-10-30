CC=gcc

CFLAGS=-Wall -std=c99 -DMEMWATCH -DMW_STDIO -c -D_POSIX_SOURCE -lcrypto 

CFLAGS_DEBUG=-Wall -std=c99 -DMEMWATCH -DMW_STDIO -c -g -DDEBUG -D_POSIX_SOURCE -lcrypto 

all: FSchecksum

debug: FSchecksum_debug

FSchecksum_debug: FSchecksum_db.o logging_db.o circ_buff_db.o memwatch_db.o
	$(CC) -pthread $^ -o $@

FSchecksum: FSchecksum.o logging.o circ_buff.o memwatch.o
	$(CC) -pthread $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $^ 

%_db.o: %.c
	$(CC) $(CFLAGS_DEBUG) -o $@ $^ 

test:

clean:
	rm *.o FSchecksum FSchecksum_debug

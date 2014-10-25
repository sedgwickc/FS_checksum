CC=gcc

CFLAGS=-Wall -std=c99 -DMEMWATCH -DMW_STDIO -g -lcrypto -D_POSIX_SOURCE

all: checksum checksum_popen checksum_memmap

%: %.c
	$(CC) $(CFLAGS) $^ -o $@

test:

clean:
	rm *.o checksum checksum_memmap checksum_popen

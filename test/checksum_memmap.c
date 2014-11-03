/* Charles Sedwick
 * 1687807
 * The following was referenced in writing this code:
 * http://beej.us/guide/bgipc/output/html/multipage/mmap.html
 */

#define _POSIX_C_SOURCE 2

#include <openssl/sha.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <sys/mman.h>
#include <time.h>

int main(int argc, char *argv[]){

	int fd, status;
	long s_page = sysconf(_SC_PAGESIZE);
	unsigned char *data;
	struct stat s_file;
	clock_t time; 
	clock_t total = 0;

	SHA_CTX c;
	unsigned char md[ SHA_DIGEST_LENGTH ];
	
	if(argc != 2){
		printf("Usage: ./checksum_memmap FILE\n");
		return 1;
	}


	for( int i = 0; i < 10; i++ ){
		fd = open(argv[1], O_RDONLY );
		assert( fd != -1);

		status = stat(argv[1], &s_file);
		assert( status != -1 );

		data = mmap(0, s_file.st_size, PROT_READ, MAP_SHARED, fd, 0);
		assert ( data != MAP_FAILED );
		
		time = clock();

		SHA1_Init(&c);

		/* why does update need num_bytes rather than S_BUF?
	 	 * if using S_BUF in update, it must be set to 160
	 	 */
		SHA1(data, s_file.st_size,md);

		total += clock() - time;
		
		munmap(data, s_page);
	}
	
	printf("The average time over 10 runs is: %f seconds.\n", ( (float)total / CLOCKS_PER_SEC ) / 10 );


	return 0;
}

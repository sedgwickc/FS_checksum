/* Charles Sedgwick
 * 1684807
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

/* manipulated variable */
#define S_BUF 1024

int main(int argc, char* argv[]){

	/* printf("Checksum\n--------\n"); */

	/* checksum code using openssl/sha.h funcs */
	SHA_CTX c;
	unsigned char buffer[ S_BUF];
	unsigned char md[ SHA_DIGEST_LENGTH ];
	int fd;
	int num_bytes = 0;
	clock_t time;
	clock_t total = 0;
	
	if( argc == 2){
		for( int i = 0; i < 10; i++ ){

			fd = open(argv[1], O_RDONLY);
			assert( fd != -1 );
			time = clock();

			SHA1_Init(&c);

			while ( (num_bytes = read( fd, buffer, S_BUF)) != 0 ){
				SHA1_Update( &c, buffer, num_bytes );
			}

			SHA1_Final( md, &c );
			/*removing assignment "time = clock() - time" shaved off a tenth of a second
 	 		 * from run time
 	 		 */
			total += clock() - time;
		}
	} else {
		printf("Usage: checksum FILE\n");
		return 0;
	}


	printf("Average time over 10 runs: %f seconds\n", ( (float)total / CLOCKS_PER_SEC ) / 10 );

	return 0;
}

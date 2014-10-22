/* Charles Sedgwick
 * 1684807
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <openssl/sha.h>
//#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define S_BUF 128

int main(int argc, char* argv[]){

	/* printf("Checksum\n--------\n"); */

	/* checksum code using openssl/sha.h funcs */
	SHA_CTX c;
	unsigned char buffer[ S_BUF + 1];
//	memset (buffer, '\0', S_BUF + 1);
	unsigned char md[ SHA_DIGEST_LENGTH ];
	int fd;
	int num_bytes = 0;
	
	if( argc == 2){
		fd = open(argv[1], O_RDONLY);
//		assert( fd != -1 );
	} else {
		printf("Usage: checksum [-n num_workers] FILE\n");
		return 0;
	}

	SHA1_Init(&c);

	/* why does update need num_bytes rather than S_BUF?
	 * if using S_BUF in update, it must be set to 160
	 */
	while ( (num_bytes = read( fd, buffer, S_BUF)) != 0 ){
		SHA1_Update( &c, buffer, num_bytes );
		//memset (buffer, '\0', S_BUF );
	}

	SHA1_Final( md, &c );

	printf("The SHA1 chechsum of %s is ", argv[1]);
	for ( int i = 0; i < SHA_DIGEST_LENGTH; i++){
		printf("%x", md[i]);
	}
	printf("\n");

	return 0;
}

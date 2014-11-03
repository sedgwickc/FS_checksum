/* Charles Sedgwick
 * 1684807
*/

/* This define explained here:
 * http://www.gnu.org/software/libc/manual/html_node/Feature-Test-Macros.html
 */
#define _POSIX_C_SOURCE 2

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>


#define S_CHECK 128

int main(int argc, char* argv[]){

	FILE *pd;
	char cmdline[64] = "sha1sum ";
	char checksum[S_CHECK];
	int rv;
	clock_t time;
	clock_t total = 0;

	if( argc != 2){
		printf("Usage: checksum FILE\n");
		return 0;
	}

	strcat(cmdline, argv[1]);
	for (int i = 0; i < 10; i++ ){

		time = clock();

		pd = popen(cmdline, "r");
		fgets(checksum, S_CHECK, pd);

		if ( (rv = pclose(pd)) == -1 ){
			printf("checksum_popen.c:: pclose() returned an error.\n");
		}

		total += clock() - time;
	}

	printf("The average time over 10 runs is: %f seconds.\n", ( (float)total / CLOCKS_PER_SEC ) / 10 );

	return 0;
}

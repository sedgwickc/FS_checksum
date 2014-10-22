/* Charles Sedgwick
 * 1684807
 *
 *
 */
#define _POSIX_C_SOURCE 2
#include <stdio.h>
#include <stdlib.h>
//#include <assert.h>
#include <unistd.h>
#include <string.h>

#define S_CHECK 128

int main(int argc, char* argv[]){

	FILE *pd;
	char cmdline[64] = "sha1sum ";
	char checksum[S_CHECK];
	int rv;

	if( argc == 2){
		strcat(cmdline, argv[1]);
		printf("cmd line: %s\n", cmdline);
	} else {
		printf("Usage: checksum [-n num_workers] FILE\n");
		return 0;
	}

	pd = popen(cmdline, "r");
	fgets(checksum, S_CHECK, pd);

	if ( (rv = pclose(pd)) == -1 ){
		printf("checksum_popen.c:: pclose() returned an error.\n");
	}

	printf("%s\n", checksum);


	return 0;
}

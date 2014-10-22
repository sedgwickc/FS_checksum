/*
 * Charles Sedgwick
 * 1684807
 * sedgwickc
 */

#include <stdio.h>
#include <assert.h>
#include "memwatch.h"

int main(int argc, char* argv[]){


	printf("\nFSchecksum\n");
	printf("----------\n");
	
	int num_worker;
	num_worker = 0;

	/* getopt code based off of example code found in getopt man page */
	while( (opt = getopt(argc, argv, "n:")) != -1 ){
		switch (opt){
			case 'n':
				num_worker = atoi(optarg);
				printf("The number of requested workers is: %d\n", num_worker);
				break;
			default:
				printf("Usage: FSchecksum [-n num_workers] FILE\n");
				break;
		}
	}

	/* accessing non-option command line arguments base on wikipedia C example
	 * found here: http://en.wikipedia.org/wiki/Getopt
	 */

	if( optind < argc ){
		printf("Non-option cmd line args:\n");
		while( optind < argc ){
			printf("%s, ", argv[optind]);
			optind++;
		}
		printf("\n");
	}
	
	return 0;
}

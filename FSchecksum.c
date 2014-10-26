/*
 * Charles Sedgwick
 * 1684807
 * sedgwickc
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <getopt.h>
#include "fschksum_gen.h"
#include "logging.h"
#include "memwatch.h"

int main(int argc, char* argv[]){


	printf("\nFSchecksum\n");
	printf("----------\n");
	
	char *log_mess = (char *)calloc(S_LOGMESS, sizeof(char));

	int num_worker;
	int opt;
	num_worker = 0;

	log_create();

	/* getopt code based off of example code found in getopt man page */
	while( (opt = getopt(argc, argv, "n:")) != -1 ){
		switch (opt){
			case 'n':
				num_worker = atoi(optarg);
				strncat(log_mess, "Running with ", S_LOGMESS);
				strncat(log_mess, optarg, S_LOGMESS);
				strncat(log_mess, " threads. \n", S_LOGMESS);
				log_write(LOG_INFO, log_mess);
				break;
			default:
				printf("Usage: FSchecksum -n NUM_WORKERS FILE\n");
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

	free(log_mess);
	return 0;
}

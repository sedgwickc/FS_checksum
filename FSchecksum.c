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
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "bound_buff.h"
#include "logging.h"
#include "memwatch.h"

int main(int argc, char* argv[]){

	char *log_mess = (char *)calloc(S_LOGMESS, sizeof(char));
	int num_workers;
	int opt;

	log_init();
#ifdef DEBUG
	strncpy( log_mess, "Verbose logging set.", S_LOGMESS );
	log_write( LOG_VERB, log_mess );
#endif

	/* getopt code based off of example code found in getopt man page */
	while( (opt = getopt(argc, argv, "n:")) != -1 ){
		switch (opt){
			case 'n':
				if( (num_workers = atoi(optarg)) > 0 ){
					strncpy(log_mess, "Running with ", S_LOGMESS);
					strncat(log_mess, optarg, S_LOGMESS);
					strncat(log_mess, " threads.", S_LOGMESS);
					log_write(LOG_INFO, log_mess);
				} else {
					printf("Cannot have 0 workers\n");
					return 1;
				}
				break;
			default:
				printf("Usage: FSchecksum -n NUM_WORKERS FILE\n");
				free(log_mess);
				return 1;
				break;
		}
	}

	if(optind == argc ){
		printf("Usage: FSchecksum -n NUM_WORKERS FILE\n");
		free(log_mess);
		return 1;
	}

	pthread_t workers[num_workers];
	buff_init(num_workers);
	
	for( int i = 0; i < num_workers; i++){
		workers[i] = buff_add_worker((long)i);
	}

	int fd, status, offset;
	char *data;
	struct stat s_file;
	File *file = malloc(sizeof(File));
	file->checksum = calloc(S_CHKSUM + 1, sizeof(unsigned char));
	file->filename = calloc(S_FPATH + 1, sizeof(char));

	fd = open(argv[optind], O_RDONLY );
	assert( fd != -1);

	status = stat(argv[optind], &s_file);
	assert( status != -1 );

	data = mmap(0, s_file.st_size, PROT_READ, MAP_SHARED, fd, 0);
	assert ( data != MAP_FAILED );

	while(  sscanf(data, "%s %s\n%n", file->checksum, file->filename, &offset) != EOF ){
		data += offset; 
#ifdef DEBUG
		printf("offset: %d data:%p\n", offset, data);
#endif
		produce(file);
	}

	buff_pdone();
	
	for( int i = 0; i < num_workers; i++){
		pthread_join(workers[i], (void *)&status );
		if( status != 0 ){
			strncpy( log_mess, strerror( status ), S_LOGMESS );
			log_write( LOG_ERR, log_mess );
		}
	}

	free(file->checksum);
	free(file->filename);
	free(file);
	buff_free();
	free(log_mess);
	log_close();
	return 0;
}

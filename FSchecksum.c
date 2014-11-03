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

	/* bounded buffer
	 * references:
	 * http://www.cs.mtu.edu/~shene/NSF-3/e-Book/SEMA/TM-example-buffer.html
	 * https://computing.llnl.gov/tutorials/pthreads/
	 *
	 * Upon creation of a thread, a Thread struct is created and populated with
	 * information regarding that thread. this information will be used for
	 * logging.
	 *
	 * main thread is the producer  
	 * -> it places pointers to File struct into the buffer
	 *
	 * the worker threads are the consumers 
	 * -> each worker grabs a File* from the buffer and starts to generate
	 *  the checksum of the file 
	 * -> once complete, it determines if the checksum is valid and marks the
	 *  buffer slot as free for the producer to fill again
	 */

	char *log_mess = (char *)calloc(S_LOGMESS, sizeof(char));
	int num_workers;
	int opt;

	log_create();
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
		/* create a Thread struct for each thread and add it to a global list of
		 * threads defined in bound_buff api
		 */
		workers[i] = buff_add_worker(i);
	}

	int fd, status, offset;
	char *data;
	struct stat s_file;
	File *file = malloc(sizeof(File));
	file->checksum = calloc(40 + 1, sizeof(unsigned char));
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
	}

	free(file->checksum);
	free(file->filename);
	free(file);
	buff_free();
	free(log_mess);
	return 0;
}

/* Charles Sedgwick
 * 1684807
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <errno.h>
#include "bound_buff.h"
#include "logging.h"
#include "memwatch.h"

File **buffer;
Thread **threads;
int num_workers;
char *log_mess;
int fillptr;
int useptr; 
int numfill;
int pdone;

pthread_mutex_t l_bbuff;
pthread_cond_t cv_remove;
pthread_cond_t cv_fill;

int buff_init(int nw){

	num_workers = nw;

	pthread_mutex_init( &l_bbuff, NULL );	
	pthread_cond_init( &cv_remove, NULL );
	pthread_cond_init( &cv_fill, NULL );

	buffer = malloc(sizeof(File*) * S_BBUFF );
	assert( buffer != NULL );
	for( int i = 0; i < S_BBUFF; i++){
		buffer[i] = malloc( sizeof(File) );
		assert( buffer[i] != NULL );
		
		buffer[i]->filename = calloc( S_FPATH + 1, sizeof(char) );
		assert( buffer[i]->filename != NULL );

		buffer[i]->checksum = calloc( S_CHKSUM, sizeof(char) );
		assert( buffer[i]->checksum != NULL );
	}

	threads = malloc( sizeof(Thread*) * num_workers );
	assert( threads != NULL );
	for( int i = 0; i < num_workers; i++ ){
		threads[i] = malloc( sizeof(Thread) );
		assert( threads[i] != NULL );

		threads[i]->data = malloc( sizeof(File) );
		assert( threads[i]->data != NULL );

		threads[i]->data->filename = calloc( S_FPATH + 1, sizeof(char) );
		assert( threads[i]->data->filename != NULL );

		threads[i]->data->checksum = calloc( S_CHKSUM, sizeof(char) );
		assert( threads[i]->data->checksum != NULL );
	}
	
	log_mess = calloc(S_LOGMESS, sizeof(char) );

	fillptr = 0;
	useptr = 0;
	numfill = 0;
	pdone = 0;
	return 0;
}

void buff_fill(File *data){
	pthread_mutex_lock( &l_bbuff );
	strncpy(buffer[fillptr]->filename, data->filename, S_FPATH );
	strncpy(buffer[fillptr]->checksum, data->checksum, S_CHKSUM );
	numfill++;
	fillptr = (fillptr + 1) % S_BBUFF;
	pthread_mutex_unlock( &l_bbuff );
}

void buff_get(File **file){
	*file = malloc( sizeof(File) );
	assert( *file != NULL );

	(*file)->filename = calloc(S_FPATH + 1, sizeof(char));
	assert( (*file)->filename != NULL);

	(*file)->checksum = calloc(S_CHKSUM, sizeof(char) );
	assert( (*file)->checksum != NULL);

	strncpy( (*file)->filename, buffer[useptr]->filename, S_FPATH );
	strncpy( (*file)->checksum, buffer[useptr]->checksum, S_CHKSUM );
	useptr = (useptr + 1) % S_BBUFF;
	numfill--;
}

void produce(File *data){
	pthread_mutex_lock( &l_bbuff );
	while(numfill == S_BBUFF){
		pthread_cond_wait( &cv_remove, &l_bbuff );
	}
	pthread_mutex_unlock( &l_bbuff );
	buff_fill( data );
	pthread_mutex_lock( &l_bbuff );
	pthread_cond_signal( &cv_fill );
	pthread_mutex_unlock( &l_bbuff );
}

/* need condition that will allow threads to break out of while loop 
 * Need to be aable to handle situation where the number of consumers is greater
 * than the number of slots in the buffer as this will cause (# consumers - #
 * buff slots) to be stuck sleeping forever on numfill == 0
 * */
void *consume(void *arg){
	while( 1 ){
		pthread_mutex_lock( &l_bbuff );
		while ( numfill == 0 && pdone == 0 ){
			pthread_cond_wait( &cv_fill, &l_bbuff );
		}
		File *file = NULL;
		buff_get(&file);
		pthread_mutex_unlock( &l_bbuff );
		/* thread connot end while having lock or no other thread will be able
		 * to grab it
		 */
		if( strncmp( file->filename, "DONE", S_FPATH ) == 0){
			free(file->filename);
			free(file->checksum);
			free(file);
			return NULL;
		}
		buff_proc(file);
#ifdef DEBUG
		printf("File: %s, numfill{C}: %d\n", file->filename, numfill);
#endif
		pthread_mutex_lock( &l_bbuff );
		pthread_cond_signal( &cv_remove );
		pthread_mutex_unlock( &l_bbuff );
		free(file->filename);
		free(file->checksum);
		free(file);
	}
	return NULL;
}

void buff_proc( File *file){

	struct stat s_file;
	int status;

#ifdef DEBUG
	strncpy(log_mess, "Thread NUM checking ", S_LOGMESS);
	strncat(log_mess, file->filename, S_LOGMESS);
	strncat(log_mess, ".", S_LOGMESS);
	log_write( LOG_VERB, log_mess); 
#endif

	status = stat( file->filename, &s_file );
	if( status == -1 ){
#ifdef DEBUG
		strncpy(log_mess, "Unable to stat ", S_LOGMESS);
		strncat(log_mess, file->filename, S_LOGMESS);
		strncat(log_mess, " -> ", S_LOGMESS); 
		strncat(log_mess, strerror(errno), S_LOGMESS); 
		log_write( LOG_ERR, log_mess); 
#endif
		return;
	}
}

 /* This only works with on producer :(
  *
  * if # consumers > # buff slots then there will be consumers sleeping waiting
  * to get a signal from the producer that there are new elements to be consumed
 */
void buff_pdone(){
	File done = {"DONE", "DONE"};
	pthread_mutex_lock( &l_bbuff );
	while ( numfill > 0 ){
		pthread_cond_wait( &cv_remove, &l_bbuff );
	}
	pthread_mutex_unlock( &l_bbuff );
	for(int i = 0; i < S_BBUFF; i++){
		produce(&done);
	}
	pthread_mutex_lock( &l_bbuff );
	pdone++;
	pthread_cond_broadcast( &cv_fill );
	pthread_mutex_unlock( &l_bbuff );
}

void buff_free(){
	
	pthread_mutex_destroy(&l_bbuff);

	for(int i = 0; i < S_BBUFF; i++){
		free(buffer[i]->filename);
		free(buffer[i]->checksum);
		free(buffer[i]);
	}

	for( int i = 0; i < num_workers; i++ ){
		free(threads[i]->data->checksum);
		free(threads[i]->data->filename);
		free(threads[i]->data);
		free(threads[i]);
	}
	
	free(log_mess);
	free(buffer);
	free(threads);
	pthread_cond_destroy( &cv_remove );
	pthread_cond_destroy( &cv_fill );
}

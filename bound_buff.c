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

char **buffer;
char *log_mess;
int fillptr;
int useptr; 
int numfill;
int pdone;

pthread_mutex_t l_bbuff;
pthread_cond_t cv_remove;
pthread_cond_t cv_fill;

int buff_init(){

	pthread_mutex_init( &l_bbuff, NULL );	
	pthread_cond_init( &cv_remove, NULL );
	pthread_cond_init( &cv_fill, NULL );

	buffer = malloc(sizeof(char*) * S_BBUFF );
	assert( buffer != NULL );
	for( int i = 0; i < S_BBUFF; i++){
		buffer[i] = calloc(S_FPATH + 1, sizeof(char) );
		assert( buffer[i] != NULL );
	}
	
	log_mess = calloc(S_LOGMESS, sizeof(char) );

	fillptr = 0;
	useptr = 0;
	numfill = 0;
	pdone = 0;
	return 0;
}

void buff_fill(char *new_file){
	pthread_mutex_lock( &l_bbuff );
	strncpy(buffer[fillptr], new_file, S_FPATH);
	numfill++;
	fillptr = (fillptr + 1) % S_BBUFF;
	pthread_mutex_unlock( &l_bbuff );
}

void buff_get(char **path){
	*path = calloc(S_FPATH + 1, sizeof(char));
	assert(*path != NULL);
	strncpy(*path, buffer[useptr], S_FPATH);
	useptr = (useptr + 1) % S_BBUFF;
	numfill--;
}

void produce(char *file_name){
	pthread_mutex_lock( &l_bbuff );
	while(numfill == S_BBUFF){
		pthread_cond_wait( &cv_remove, &l_bbuff );
	}
	pthread_mutex_unlock( &l_bbuff );
	buff_fill(file_name);
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
		char *item = NULL;
		buff_get(&item);
		pthread_mutex_unlock( &l_bbuff );
		/* thread connot end while having lock or no other thread will be able
		 * to grab it
		 */
		if( strncmp( item, "DONE", S_BBUFF ) == 0){
			free(item);
			return NULL;
		}
		buff_proc(item);
#ifdef DEBUG
		printf("Item: %s, numfill{C}: %d\n", item, numfill);
#endif
		pthread_mutex_lock( &l_bbuff );
		pthread_cond_signal( &cv_remove );
		pthread_mutex_unlock( &l_bbuff );
		free(item);
	}
	return NULL;
}

void buff_proc( char *item){

	struct stat s_file;
	int status;

#ifdef DEBUG
	strncpy(log_mess, "Thread NUM checking ", S_LOGMESS);
	strncat(log_mess, item, S_LOGMESS);
	strncat(log_mess, ".", S_LOGMESS);
	log_write( LOG_VERB, log_mess); 
#endif

	status = stat( item, &s_file );
	if( status == -1 ){
#ifdef DEBUG
		strncpy(log_mess, "Unable to stat ", S_LOGMESS);
		strncat(log_mess, item, S_LOGMESS);
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
	pthread_mutex_lock( &l_bbuff );
	while ( numfill > 0 ){
		pthread_cond_wait( &cv_remove, &l_bbuff );
	}
	pthread_mutex_unlock( &l_bbuff );
	for(int i = 0; i < S_BBUFF; i++){
		produce("DONE");
	}
	pthread_mutex_lock( &l_bbuff );
	pdone++;
	pthread_cond_broadcast( &cv_fill );
	pthread_mutex_unlock( &l_bbuff );
}

void buff_free(){
	
	pthread_mutex_destroy(&l_bbuff);

	for(int i = 0; i < S_BBUFF; i++){
		free(buffer[i]);
	}
	free(log_mess);
	free(buffer);
	pthread_cond_destroy( &cv_remove );
	pthread_cond_destroy( &cv_fill );
}

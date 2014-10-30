/* Charles Sedgwick
 * 1684807
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include "circ_buff.h"
#include "memwatch.h"

char **buffer;
int fillptr;
int useptr; 
int numfill;

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
	fillptr = 0;
	useptr = 0;
	numfill = 0;
	return 0;
}

void buff_fill(char *new_file){
	strncpy(buffer[fillptr], new_file, S_FPATH);
	fillptr = (fillptr + 1) % S_BBUFF;
	pthread_mutex_lock(&l_bbuff);
	numfill++;
	pthread_mutex_unlock(&l_bbuff);
}

void buff_get(char **path){
	*path = calloc(S_FPATH + 1, sizeof(char));
	assert(*path != NULL);
	strncpy(*path, buffer[useptr], S_FPATH);
	useptr = (useptr + 1) % S_BBUFF;
	pthread_mutex_lock( &l_bbuff );
	numfill--;
	pthread_mutex_unlock( &l_bbuff );
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
 * use a semaphore?
 * http://www.csc.villanova.edu/~mdamian/threads/posixsem.html
 * */
void *consume(void *arg){
	while( 1 ){
		pthread_mutex_lock( &l_bbuff );
		while ( numfill == 0 ){
			/* sem_value may change before wait and thus consumer will never
			 * wake up
			 *
			 * look at textbook for soln
			 */
			pthread_cond_wait( &cv_fill, &l_bbuff );
		}
		pthread_mutex_unlock( &l_bbuff );
		char *item = NULL;
		buff_get(&item);
		if( strncmp( item, "DONE", S_BBUFF ) == 0){
			free(item);
			return NULL;
		}
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

/* bug: sem_pdone is not incremented after main thread done producing or 
 * consumers go to sleep waiting on cv_fill and never wake up b/c main thread is
 * done producing
 *
 * This only works with on producer :(
 */
void buff_pdone(){
	pthread_mutex_lock( &l_bbuff );
	while ( numfill > 0 ){
		pthread_cond_wait( &cv_remove, &l_bbuff );
	}
	pthread_mutex_unlock( &l_bbuff );
	/* consumers can grab data already processed before producer can set buffer
	 * entries to done
	 */
	for(int i = 0; i < S_BBUFF; i++){
		produce("DONE");
	}
}

void buff_free(){
	
	pthread_mutex_destroy(&l_bbuff);

	for(int i = 0; i < S_BBUFF; i++){
		free(buffer[i]);
	}

	free(buffer);
}

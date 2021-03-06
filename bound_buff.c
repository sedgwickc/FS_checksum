/* Charles Sedgwick
 * 1684807
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <errno.h>
#include <openssl/sha.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include "bound_buff.h"
#include "logging.h"
#include "memwatch.h"

File **buffer;
Thread **workers;
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

	num_workers = (int)nw;

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

		buffer[i]->checksum = calloc( S_CHKSUM + 1, sizeof(char) );
		assert( buffer[i]->checksum != NULL );
	}

	workers = malloc( sizeof(Thread*) * num_workers );
	assert( workers != NULL );
	for( int i = 0; i < num_workers; i++ ){
		workers[i] = malloc( sizeof(Thread) );
		assert( workers[i] != NULL );

		workers[i]->data = malloc( sizeof(File) );
		assert( workers[i]->data != NULL );

		workers[i]->data->filename = calloc( S_FPATH + 1, sizeof(char) );
		assert( workers[i]->data->filename != NULL );

		workers[i]->data->checksum = calloc( S_CHKSUM + 1, sizeof(char) );
		assert( workers[i]->data->checksum != NULL );

		workers[i]->num_files = 0;
	}
	
	log_mess = calloc(S_LOGMESS, sizeof(char) );

	fillptr = 0;
	useptr = 0;
	numfill = 0;
	pdone = 0;
	return 0;
}

pthread_t buff_add_worker(long index){
		pthread_t tid;
		pthread_create(&tid, NULL, consume, (void *)index);
		workers[index]->thread_id = tid;
		return tid;
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

	assert( *file != NULL);
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

void *consume(void *arg){

	char t_id[20];
	char t_files[20];
	File *file = malloc( sizeof(File) );
	assert( file != NULL );

	file->filename = calloc(S_FPATH + 1, sizeof(char));
	assert( file->filename != NULL);

	file->checksum = calloc(S_CHKSUM + 1, sizeof(char) );
	assert( file->checksum != NULL);
	
	while( 1 ){
		pthread_mutex_lock( &l_bbuff );
		while ( numfill == 0 && pdone == 0 ){
			pthread_cond_wait( &cv_fill, &l_bbuff );
		}
		memset(file->filename, 0, S_FPATH );
		memset(file->checksum, 0, S_CHKSUM );
		buff_get(&file);
		pthread_mutex_unlock( &l_bbuff );
		if( strncmp( file->filename, "DONE", S_FPATH ) == 0 ){
			if( file != NULL ){
				free( file->filename );
				free( file->checksum );
				free( file );
			}
			sprintf( t_id, "%u", (unsigned)workers[(long)arg]->thread_id );
			strncpy( log_mess, "Thread ", S_LOGMESS );
			strncat( log_mess, t_id, S_LOGMESS );
			strncat( log_mess, " processed ", S_LOGMESS );
			sprintf( t_files, "%d", workers[(long)arg]->num_files );
			strncat( log_mess, t_files, S_LOGMESS );
			strncat( log_mess, " files", S_LOGMESS );
			log_write( LOG_INFO, log_mess );
			return NULL;
		}
		buff_proc( file );
		workers[(long)arg]->num_files++;
#ifdef DEBUG
		printf("t_index: %ld, fileno: %d\n", (long)arg, workers[(long)arg]->num_files);
#endif
		pthread_mutex_lock( &l_bbuff );
		pthread_cond_signal( &cv_remove );
		pthread_mutex_unlock( &l_bbuff );
	}
	return NULL;
}

void buff_proc( File *file){

	struct stat s_file;
	char t_num[20];
	unsigned char *checksum = calloc( SHA_DIGEST_LENGTH, sizeof( unsigned char ) );
	int status;
	pthread_t id = pthread_self();
	sprintf( t_num, "%u", (unsigned)id );

#ifdef DEBUG
	strncpy( log_mess, "Thread ", S_LOGMESS );
	strncat( log_mess, t_num, S_LOGMESS );
	strncat( log_mess, " checking ", S_LOGMESS );
	strncat( log_mess, file->filename, S_LOGMESS );
	log_write( LOG_VERB, log_mess );
#endif

	status = stat( file->filename, &s_file );
	if( status == -1 ){
#ifdef DEBUG
		strncpy(log_mess, "Unable to stat ", S_LOGMESS);
		strncat(log_mess, file->filename, S_LOGMESS);
		strncat(log_mess, " -> ", S_LOGMESS); 
		strncat(log_mess, strerror(errno), S_LOGMESS); 
		log_write( LOG_ERR, log_mess); 
		free(checksum);
#endif
		return;
	}

	gen_checksum( file->filename, checksum );
	
	char curr_checksum[S_CHKSUM + 1];
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++){
		snprintf( curr_checksum+i*2, 3, "%02x", checksum[i] );
	}
	if( strncmp( file->checksum, curr_checksum, S_CHKSUM ) != 0){
		strncpy(log_mess, "Mismatch detected ", S_LOGMESS);
		strncat(log_mess, file->filename, S_LOGMESS);
		strncat(log_mess, " got ", S_LOGMESS);
		strncat(log_mess, curr_checksum, S_LOGMESS);
		strncat(log_mess, " should be ", S_LOGMESS);
		strncat(log_mess, file->checksum, S_LOGMESS);
		log_write( LOG_WARN, log_mess); 
	}

#ifdef DEBUG
	printf("chksum_orig: %s, chksum_curr: %s\n", file->checksum, curr_checksum );
#endif

	free( checksum );
}

void gen_checksum(char *filepath, unsigned char *checksum){
	int fd, status;
	long s_page = sysconf(_SC_PAGESIZE);
	unsigned char *data;
	struct stat s_file;

	SHA_CTX c;
	unsigned char md[ SHA_DIGEST_LENGTH ];
	
	status = stat(filepath, &s_file);
	assert( status != -1 );

	fd = open( filepath, O_RDONLY );
	assert( fd != -1);

	data = mmap( 0, s_file.st_size, PROT_READ, MAP_SHARED, fd, 0);
	assert ( data != MAP_FAILED );
	
	SHA1_Init(&c);

	SHA1(data, s_file.st_size,md);

	for(int i = 0; i < SHA_DIGEST_LENGTH; i++){
		checksum[i] = md[i];
	}

	munmap(data, s_page);
	close(fd);
}

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
	
	pthread_mutex_destroy( &l_bbuff );

	for(int i = 0; i < S_BBUFF; i++){
		free(buffer[i]->filename);
		free(buffer[i]->checksum);
		free(buffer[i]);
	}

	for( int i = 0; i < num_workers; i++ ){
		free(workers[i]->data->checksum);
		free(workers[i]->data->filename);
		free(workers[i]->data);
		free(workers[i]);
	}
	
	free( log_mess );
	free( workers );
	free( buffer );
	pthread_cond_destroy( &cv_remove );
	pthread_cond_destroy( &cv_fill );
}

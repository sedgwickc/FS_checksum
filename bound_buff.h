#ifndef CIRC_BUFF_H
#define CIRC_BUFF_H

/* Charles Sedgwick
 * 1684807
 * References:
 * http://beej.us/guide/bgipc/output/html/multipage/semaphores.html
 */

#define _XOPEN_SOURCE 699 
#define S_BBUFF 5
#define S_FPATH 64
#define S_CHKSUM 40

#include <unistd.h>
#include <pthread.h>
#include <openssl/sha.h>

typedef struct {
	char *filename;
	char *checksum;
}File;

typedef struct {
	pthread_t thread_id;
	size_t bytes_read;
	int num_files;
	File *data;
}Thread;

extern File **buffer;
extern Thread **threads;
extern int num_workers;
extern int fillptr;
extern int useptr;
extern int numfill;

extern pthread_mutex_t l_bbuff;
extern pthread_cond_t cv_remove;
extern pthread_cond_t cv_fill;

int buff_init(int num_workers);

pthread_t buff_add_worker(int index);

void buff_fill( File *f );

void buff_get( File **data );

void produce();

void *consume( void * );

void buff_proc( File *f);

void buff_pdone();

void buff_free();

#endif

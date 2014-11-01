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
	char *file_name;
	char *checksum;
}File;

extern char **buffer;
extern int fillptr;
extern int useptr;
extern int numfill;

extern pthread_mutex_t l_bbuff;
extern pthread_cond_t cv_remove;
extern pthread_cond_t cv_fill;

int buff_init();

void buff_fill( char *new_file );

void buff_get( char **item );

void produce();

void *consume( void * );

void buff_pdone();

void buff_free();

#endif

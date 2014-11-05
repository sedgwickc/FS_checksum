#ifndef LOGGING_H
#define LOGGING_H

#include <pthread.h>

#define S_ERRMESS 128
#define S_LOGMESS 128
#define S_LPATH 128

#define LOG_PATH "/tmp/FSchksum.log"
#define LOG_VAR "FSCHECKSUMLOG"
#define LOG_INFO 0
#define LOG_ACT 1
#define LOG_WARN 2
#define LOG_ERR 3
#define LOG_DEBUG 4
#define LOG_VERB 5

const char *log_types[6];
extern pthread_mutex_t l_log;

void log_init();

char *log_path();

void log_write(int type, char *mess);

void log_create();

void log_close();

#endif

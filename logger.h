#ifndef __LOGGER_H__
#define __LOGGER_H__
#include <sys/file.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <pthread.h>

#define TRUE	1
#define FALSE	0
#define LINE_LENGTH 1024

/**
 * @brief: callback function used in logger setting
 * @details: lock function for multi-thread 
 * @param udata: mutex or rwlock etc.
 * @param lock: 0: unlock, 1: lock
 */
typedef void (*log_LockFn)(void* udata, int lock);

/**
 * @brief: try to get file lock
 * @details: no block to get file lock
 * @param fd: file handle
 * @return TRUE: get lock; FLASE: get no lock
 * @attention: this lock just is a ...lock
 */
int trylock_fd(int fd);

/**
 * @brief: try to get a file lock or wait
 * @details: block...
 */
int waitlock_fd(int fd);


/**
 * @brief: release file lock
 */
int unlock_fd(int fd);


enum { LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL };

#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

void log_set_udata(void *udata);
void log_set_lock(log_LockFn fn);
void log_set_fp(FILE *fp);
void log_set_level(int level);
void log_set_quiet(int enable);

void log_log(int level, const char *file, int line, const char *fmt, ...);

//int record_log(const char* filename, const char* str);
//int record_log(FILE* fp, const char* str);

void init_logger(void);
void destroy_logger(void);
#endif

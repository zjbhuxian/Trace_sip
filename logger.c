#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "../../sr_module.h"

#include "logger.h"

pthread_rwlock_t rwlock;

void my_rwlock(void* udata, int lock)
{
	if(lock == 1){
		pthread_rwlock_wrlock((pthread_rwlock_t*)udata);
	}else if(lock == 0){
		pthread_rwlock_unlock((pthread_rwlock_t*)udata);
	}else{
		fprintf(stderr, "Invalid lock flag\n");
	}
}

static struct {
	void*				udata;
	log_LockFn	lock;
	FILE*				fp;
	int					level;
	int					quiet;
}L;

static const char* level_names[] = {
	"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char* level_colors[] = {
	"\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif

static void lock(void)
{
	if(L.lock){
		L.lock(L.udata, 1);
	}
}

static void unlock(void)
{
	if(L.lock){
		L.lock(L.udata, 0);
	}
}

void log_set_udata(void* udata)
{
	L.udata = udata;
}

void log_set_lock(log_LockFn fn)
{
	L.lock = fn;
}

void log_set_fp(FILE* fp)
{
	L.fp = fp;
}

void log_set_level(int level)
{
	L.level = level;
}

void log_set_quiet(int enable)
{
	L.quiet = enable ? 1 : 0;
}

void log_log(int level, const char* file, int line, const char* fmt, ...)
{
	if(level < L.level){
		return;
	}

	/* Acquire lock */
	lock();

	/* Get current time */
	time_t t = time(NULL);
	struct tm* lt = localtime(&t);

	/* Log to stderr */
	if(!L.quiet){
		va_list args;
		char buf[16];
		buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';
#ifdef LOG_USE_COLOR
		fprintf(stderr, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
				buf, level_colors[level], level_names[level], file, line);
#else
		fprintf(stderr, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
#endif
		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
		fprintf(stderr, "\n");
		fflush(stderr);
	}

	/* Log to file */
	if(L.fp){
		va_list args;
		char buf[32];
		buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
		fprintf(L.fp, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
		va_start(args, fmt);
		vfprintf(L.fp, fmt, args);
		va_end(args);
		fprintf(L.fp, "\n");
		fflush(L.fp);
	}

	/* Log to redis */

	/* Release lock */
	unlock();
}

int trylock_fd(int fd)
{
	if(flock(fd, LOCK_EX|LOCK_NB) == 0){
		return TRUE;
	}else{
		return FALSE;
	}
}


/**
 * @brief: try to get a file lock or wait
 * @details: block...
 */
int waitlock_fd(int fd)
{
  if(flock(fd, LOCK_EX) == 0){
		return TRUE;
  }else{
		return FALSE;
  }
}

/**
 * @brief: release file lock
 */
int unlock_fd(int fd)
{
  if(flock(fd, LOCK_UN) == 0){
		return TRUE;
  }else{
		return FALSE;
  }
}

void init_logger(void)
{
	pthread_rwlock_init(&rwlock, NULL);
	log_set_udata(&rwlock);
	log_set_lock(my_rwlock);
	//log_set_fp()
	log_set_level(_LOG_TRACE);
	log_set_quiet(1);
}

void destroy_logger(void)
{
	pthread_rwlock_destroy(&rwlock);
	if(L.fp){
		fclose(L.fp);
		L.fp = NULL;
	}
}

void ct_write_to_file(const char* filename, const char* msg)
{
	if(!filename || !msg)return;
	LM_DBG("filename, msg = [%s][%s]\n", filename, msg);

	FILE* fp = fopen(filename, "a+");
	if(!fp){
		LM_ERR("Failed to open file [%s]\n", filename);
		return;
	}
	init_logger();
	log_set_fp(fp);
	log_info(msg);	
	destroy_logger();
}

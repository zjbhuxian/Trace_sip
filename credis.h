/* credis.h -- simple redis client for publishing
https://github.com/zjbhuxian/hiredis_samples
*/

#ifndef __CREDIS_H__
#define __CREDIS_H__
#include <stdio.h>
#include <stdlib.h>
#include <hiredis/adapters/libevent.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>
#include <pthread.h>
#include "../../sr_module.h"

typedef struct{
	struct event_base* e_base;
	sem_t e_sem;
}Semevent, *PSemevent;

int init_semevent(PSemevent pse);

int release_semevent(PSemevent pse);

void* event_thread(void* data);

void callback_command(redisAsyncContext* ctx, void* reply, void* privdata);

void callback_connect(const redisAsyncContext* ctx, int status);

void callback_disconnect(const redisAsyncContext* ctx, int status);

int write_to_redis(const char* host, int port, const char* ch, const char* msg);
#endif

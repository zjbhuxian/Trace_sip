#include "credis.h"
int init_semevent(PSemevent pse)
{
	if(!pse)return -1;
	int ret = 0;

	pse->e_base = event_base_new();
	if(!pse->e_base){
		LM_ERR("Failed to create redis event.\n");
		return -1;
	}

	memset(&pse->e_sem, 0x00, sizeof(pse->e_sem));
	ret = sem_init(&pse->e_sem, 0, 0);
	if(ret != 0){
		LM_ERR("Failed to initialize sem.\n");
		release_semevent(pse);
		return -1;
	}

	return 0;
}

int release_semevent(PSemevent pse)
{
	if(!pse)return -1;

	pse->e_base = NULL;
	sem_destroy(&pse->e_sem);

	return 0;
}

void* event_thread(void* data)
{
	if(!data){
		LM_ERR("Data is NULL.\n");
		return NULL;
	}

	LM_DBG("...In event_thread...\n");
	PSemevent pse = (PSemevent)data;
	sem_wait(&pse->e_sem);
	event_base_dispatch(pse->e_base);

	return NULL;
}

void callback_command(redisAsyncContext* ctx, void* reply, void* privdata)
{
	LM_DBG("Callback of Redis publish command...\n");
	*((int*)privdata) = 1; // indicating command has been executed.
}

void callback_connect(const redisAsyncContext* ctx, int status)
{
	LM_DBG("Callback of Redis connect...\n");
	if(status != REDIS_OK){
		LM_ERR("Failed to connect to redis server.\n");
	}else{
		LM_DBG("Successed to connect to redis server.\n");
	}
}

void callback_disconnect(const redisAsyncContext* ctx, int status)
{
	LM_DBG("Callback of Redis disconnect...\n");
	if(status != REDIS_OK){
		LM_ERR("Failed to disconnect to redis server.\n");
	}else{
		LM_DBG("Successed to disconnect to redis server.\n");
	}
}

int write_to_redis(const char* host, int port, const char* ch, const char* msg)
{
	if(!host || !ch || !msg || port < 0)return -1;

	int ret = 0;
	int command_finished_flag = 0;
	int timeout = 10000;
	pthread_t tid;
	Semevent se;

	ret = init_semevent(&se);
	if(ret != 0){
		LM_ERR("Failed to init semevent.\n");
		return -1;
	}

	redisAsyncContext* ctx = redisAsyncConnect(host, port);
	if(!ctx){
		LM_ERR("Failed to connect to redis server.\n");
		release_semevent(&se);
		return -1;
	}

	if(ctx->err){
		LM_ERR("Error: [%s]\n", ctx->errstr);
		release_semevent(&se);
		return -1;
	}

	redisLibeventAttach(ctx, se.e_base);
	ret = pthread_create(&tid, 0, &event_thread, (void*)&se);
	if(ret != 0){
		LM_ERR("Failed to create thread.\n");
		release_semevent(&se);
		return -1;
	}

	redisAsyncSetConnectCallback(ctx, callback_connect);
	redisAsyncSetDisconnectCallback(ctx, callback_disconnect);

	sem_post(&se.e_sem);

	ret = redisAsyncCommand(ctx, callback_command, (void*)&command_finished_flag, "PUBLISH %s %s", ch, msg);
	if(ret == REDIS_ERR){
		LM_ERR("Failed to publish...\n");
		goto Err;
	}

	while(command_finished_flag != 1 || timeout < 0){
		usleep(10);
		timeout--;
	}

Err:
	if(ctx){
		redisAsyncDisconnect(ctx);
		//redisAsyncFree(ctx);
		ctx = NULL;
	}

	release_semevent(&se);

	return 0;
}

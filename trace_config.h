#ifndef __TRACE_CONFIG_H__
#define __TRACE_CONFIG_H__
typedef struct {
	const char *host;
	const char *username;
	const char *password;
	int  port;
	const char *dbname;
	const char *table;	
}Dbconfig, *PDbconfig;

typedef struct {
	const char *dir;
	const char *suffix;
	const char *basename;
}Fileconfig, *PFileconfig;

typedef struct {
}Redisconfig, *PRedisconfig;

typedef struct {
	int		output_dest;	// indicating destination
	Dbconfig	dbconfig;	
	Fileconfig	fileconfig;	
	Redisconfig	redisconfig;
}Traceconfig, *PTraceconfig;

void init_dbconfig(PDbconfig pdbc);
void free_dbconfig(PDbconfig pdbc);
void print_dbconfig(PDbconfig pdbc);

void init_fileconfig(PFileconfig pfc);
void free_fileconfig(PFileconfig pfc);
void print_fileconfig(PFileconfig pfc);

void init_redisconfig(PRedisconfig prc);
void free_redisconfig(PRedisconfig prc);
void print_redisconfig(PRedisconfig prc);

void init_traceconfig(PTraceconfig ptc);
void free_traceconfig(PTraceconfig ptc);
void print_traceconfig(PTraceconfig ptc);

int get_config(const char *filename, PTraceconfig ptc);
#endif

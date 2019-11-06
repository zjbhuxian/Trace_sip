#include <stdio.h>
#include <string.h>
#include "tools.h"
//#include "../../sr_module.h"
#include "trace_config.h"
#include "ini.h"

const char *B_OUTPUT      = "output";
const char *OUTPUT_DEST = "output_dest";
const char *B_MYSQL       = "mysql";
const char *HOST        = "host"; 
const char *USERNAME    = "username";
const char *PASSWORD    = "password";
const char *PORT        = "port";
const char *DBNAME      = "dbname";
const char *TABLE       = "table";

const char *B_FILE     = "file";
const char *DIRSTR      = "dir";
const char *SUFFIX      = "suffix";
const char *BASENAME    = "basename";

void init_dbconfig(PDbconfig pdbc)
{
	if(!pdbc){
		printf("Failed to init PDbconfig.\n");
		return;
	}

	pdbc->host = NULL;
	pdbc->username = NULL;
	pdbc->password = NULL;
	pdbc->port = 0;
	pdbc->dbname = NULL;
}

void free_dbconfig(PDbconfig pdbc)
{
	if(!pdbc)return;

	release_pointer((void**)(&(pdbc->host)));
	release_pointer((void**)(&(pdbc->username)));
	release_pointer((void**)(&(pdbc->password)));
	release_pointer((void**)(&(pdbc->dbname)));
}

void print_dbconfig(PDbconfig pdbc)
{
	if(!pdbc)return;

  printf("host:%s\n", pdbc->host);
  printf("username:%s\n", pdbc->username);
  printf("password:%s\n", pdbc->password);
  printf("port:%d\n", pdbc->port);
  printf("dbname:%s\n", pdbc->dbname);
}

void init_fileconfig(PFileconfig pfc)
{
	if(!pfc){
		printf("Failed to init PFileconfig.\n");
		return;
	}

	pfc->dir = NULL;
	pfc->suffix = NULL;
	pfc->basename = NULL;
}

void free_fileconfig(PFileconfig pfc)
{
	if(!pfc)return;

	release_pointer((void**)(&(pfc->dir)));
	release_pointer((void**)(&(pfc->suffix)));
	release_pointer((void**)(&(pfc->basename)));
}

void print_fileconfig(PFileconfig pfc)
{
  if(!pfc)return;

  printf("dir:%s\n", pfc->dir);
  printf("suffix:%s\n", pfc->suffix);
  printf("basename:%s\n", pfc->basename);
}

void init_redisconfig(PRedisconfig prc)
{
	if(!prc){
		printf("Failed to init PRedisconfig.\n");
		return;
	}

	/* Do something */
	prc->host = NULL;
	prc->channel = NULL;
	return;
}

void free_redisconfig(PRedisconfig prc)
{
  if(!prc)return;

	release_pointer((void**)(&(prc->host)));
	release_pointer((void**)(&(prc->channel)));
}

void print_redisconfig(PRedisconfig prc)
{
}

void init_traceconfig(PTraceconfig ptc)
{
	if(!ptc){	
		printf("Failed to init PTraceconfig.\n");
		return;
	}

	ptc->output_dest = 1; // default setting	
	init_dbconfig(&(ptc->dbconfig));
	init_fileconfig(&(ptc->fileconfig));
	init_redisconfig(&(ptc->redisconfig));
}

void free_traceconfig(PTraceconfig ptc)
{
	if(!ptc)return;

	free_dbconfig(&(ptc->dbconfig));
	free_fileconfig(&(ptc->fileconfig));
	free_redisconfig(&(ptc->redisconfig));
}

void print_traceconfig(PTraceconfig ptc)
{
	if(!ptc)return;

  printf("output_dest:%d\n", ptc->output_dest);
  print_dbconfig(&(ptc->dbconfig));
  print_fileconfig(&(ptc->fileconfig));
  print_redisconfig(&(ptc->redisconfig));
}

static int handler(void* user, const char* section, const char* name, const char* value)
{
  PTraceconfig ptc = (PTraceconfig)user;
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
  if(MATCH("output", "output_dest")){
    ptc->output_dest = atoi(value);
  }else if(MATCH("mysql", "host")){
    ptc->dbconfig.host = strdup(value);
  }else if(MATCH("mysql", "username")){
    ptc->dbconfig.username = strdup(value);
  }else if(MATCH("mysql", "password")){
    ptc->dbconfig.password = strdup(value);
  }else if(MATCH("mysql", "port")){
    ptc->dbconfig.port = strtoul(value, NULL, 10);
  }else if(MATCH("mysql", "dbname")){
    ptc->dbconfig.dbname = strdup(value);
  }else if(MATCH("file", "dir")){
    ptc->fileconfig.dir = strdup(value);
  }else if(MATCH("file", "suffix")){
    ptc->fileconfig.suffix = strdup(value);
  }else if(MATCH("file", "basename")){
    ptc->fileconfig.basename = strdup(value);
	}else if(MATCH("redis", "host")){
    ptc->redisconfig.host = strdup(value);
	}else if(MATCH("redis", "port")){
    ptc->redisconfig.port= strtoul(value, NULL, 10);
	}else if(MATCH("redis", "channel")){
    ptc->redisconfig.channel= strdup(value);
  }else{
    return 0; /* unknown section/name, error */
  }

  return 1;
}

int get_config(const char *filename, PTraceconfig ptc)
{
	if(!filename){
		printf("Filename is NULL.\n");
		return -1;
	}

	FILE *fp = fopen(filename, "r");
	if(!fp){
		printf("Failed to open filename [%s].\n", filename);
		return -1;
	}	

  if(ini_parse_file(fp, handler, ptc)){
    printf("Failed to load configuration file.\n");
    return -1;
  }

  if(fp){
    fclose(fp);
    fp = NULL;
  }

	return 0;
}

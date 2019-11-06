#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmysql.h"
#include "trace_config.h"
#include "logger.h"

const char* config_file = "/home/zhou/github/mysqlFolder/trace_sip.cfg";
const char* g_logfile = NULL;
pthread_rwlock_t rwlock;
Traceconfig tc;

int main(int argc, char **argv)
{
  if(!config_file){
    printf("config_file is NULL.\n");
    return -1;
  }

	FILE*									fp = NULL;
  int                   ret = 0;
  MYSQL                 mysql;
  MYSQL*                pmysql = NULL;
  query_trace_info      qti;
  query_business_info   qbi;
  char*                 logfile = NULL;
  size_t                len = 0;
  int                   output_dest = 0;
  int                   log_flag = 0;

  init_traceconfig(&tc);

  // 1) get setting
  ret = get_config(config_file, &tc);
  if(ret != 0){
    printf("Failed to get config.\n");
    goto Err;
  }

  output_dest = tc.output_dest;
  printf("output_dest = %d\n", output_dest);

  print_traceconfig(&tc);
  // get logger filename
  len = strlen(tc.fileconfig.dir) + strlen(tc.fileconfig.basename) + strlen(tc.fileconfig.suffix);
  logfile = (char*)malloc(len + 1);
  if(!logfile){
    printf("Failed to malloc memory for logger file.\n");
    goto Err;
  }
  memset(logfile, 0x00, len+1);
  strcat(logfile, tc.fileconfig.dir);
  strcat(logfile, tc.fileconfig.basename);
  strcat(logfile, tc.fileconfig.suffix);
  logfile[len] = '\0';

  printf("logfile = [%s]\n", logfile);
  g_logfile = logfile;

  // 2) check table trace_info
  pmysql = db_connect_to_database(&mysql, tc.dbconfig.host, tc.dbconfig.username, 
                                tc.dbconfig.password, tc.dbconfig.dbname);

  if(!pmysql){
    printf("Failed to connect to database.\n");
    goto Err;
  }else{
    printf("Successed to connect to database.\n");
  }

  init_query_trace_info(&qti);
  ret = db_get_trace_info(&mysql, &qti);
  if(ret != 0){
    // create trace_info table
    init_query_business_info(&qbi);
    ret = db_get_business_info(&mysql, &qbi);
    if(ret != 0){
      printf("Failed to select data\n");
    }else{
      if(qbi.num_rows > 0){
        if(qbi.pbusinessInfo->trace_flag == 1){
          log_flag = 1;
        }
        /*else{
          log_flag = 0;
        }*/
      }
      printf("sign_num = [%s]\n", qbi.pbusinessInfo->sign_number);
      printf("sign_num = [%d]\n", qbi.pbusinessInfo->trace_flag);
    }
  }else{
    if(qti.num_rows > 0){
      log_flag = 1;
    }
  }

  if(log_flag == 1){
    switch(output_dest){
      case 1:
        printf("Log to file.\n");
				if((fp = fopen(g_logfile, "a+")) != NULL){
					log_set_fp(fp);
					log_trace("Test log...\n");
				}
        break;
      case 2:
        printf("Log to redis.\n");
        break;
      case 3:
        printf("Log to file and redis.\n");
        break;
      default:
        printf("Invalid output_dest.\n");
        break;
    }
  }

Err:
  if(pmysql){
    db_disconnect_to_database(&mysql);
    pmysql = NULL;
		destroy_logger();
  }

  return 0;
}

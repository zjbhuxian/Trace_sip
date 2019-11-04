#ifndef __CMYSQL_H__
#define __CMYSQL_H__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mysql/mysql.h>

#define LENG_20 20
#define LENG_50 50
#define LENG_100 100
#define LENG_200 200
#define LENG_256 256

typedef int (*callback)(void*, MYSQL_RES*, int, int);

typedef struct business_msg{
  char  sign_number[LENG_20];
  char  called_format[LENG_50];
  char  sign_type[LENG_50];
  char  record_flag[LENG_50];
  char  sign_code[LENG_100];
  char  custom_id[LENG_100];
  char  playback_sound_address[LENG_200];
  int   bridge_flag;
  int   valid_flag;
  int   use_origin_call;
  int   use_origin_called;
  char  call_no[LENG_50];
  char  called_no[LENG_50];
  char  display_no[LENG_50];
  char  real_called_no[LENG_50];
  char  route[LENG_100];
  char  domain[LENG_200];
  char  create_time[LENG_100];
  char  create_user[LENG_50];
  char  modify_time[LENG_100];
  char  modify_user[LENG_50];
  int   trace_flag;
}business_info, *pbusiness_info;

typedef struct query_business_info{
  int num_columns;
  int num_rows;
  int cur_row;
  pbusiness_info pbusinessInfo;
}query_business_info, *pquery_business_info;

int init_query_business_info(pquery_business_info pqbi);

typedef struct trace_table{
  char  incallid[LENG_256];
  char  outcallid[LENG_256];
}trace_info, *ptrace_info;

typedef struct query_trace_info{
  int num_columns;
  int num_rows;
  int cur_row;
  ptrace_info ptraceInfo;
}query_trace_info, *pquery_trace_info;

int init_query_trace_info(pquery_trace_info pqti);

MYSQL* db_connect_to_database(MYSQL* mysql, const char* host, const char* username,
                              const char* password, const char* dbname);

MYSQL* db_get_database_connection(MYSQL* mysql, const char* host, const char* username,
                              const char* password, const char* dbname);

int db_disconnect_to_database(MYSQL* mysql);

int db_put_database_connection(MYSQL* mysql);

int get_db_format_time_str_from_tm(struct tm* timeinfo, char *format_time);
int get_db_tm_from_format_time(char* format_time, struct tm* timeinfo);

void callback_print_query_info(void* result, MYSQL_ROW row, int num_fields, int num_rows);
int callback_get_business_info(void* arg, MYSQL_RES* resultSet, int num_fields, int num_rows);
int callback_get_trace_info(void* arg, MYSQL_RES* resultSet, int num_fields, int num_rows);

/* db interfaces, including create, delete, insert etc. */
int db_do_query_2(MYSQL* mysql, const char* query, callback cb, void* result);

/* select interfaces */
int db_get_business_info(MYSQL* mysql, pquery_business_info query_info);
int db_get_trace_info(MYSQL* mysql, pquery_trace_info query_info);

/* add table interfaces */
int db_add_trace_info(MYSQL* mysql, ptrace_info traceinfo);

/* delete table interfaces */
int db_delete_trace_info(MYSQL* mysql, ptrace_info traceinfo);

/* update table interfaces */
//int db_update_trace_info(ptrace_info traceinfo);

#endif

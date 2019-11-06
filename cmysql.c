#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include "cmysql.h"

int get_db_format_time_str_from_tm(struct tm* timeinfo, char *format_time)
{
  if(timeinfo == NULL || format_time == NULL){
  }

  return 0;
}

int get_db_tm_from_format_time(char* format_time, struct tm* timeinfo)
{
  return 0;
}

int init_business_info(pbusiness_info pbi)
{
	if(!pbi){
		return -1;
	}
	return 0;
}

int init_query_business_info(pquery_business_info pqbi)
{
	if(!pqbi){
		printf("parameter is NULL.\n");
		return -1;
	}

	pqbi->pbusinessInfo = NULL;
	return 0;
}

int init_query_trace_info(pquery_trace_info pqti)
{
	if(!pqti){
		printf("parameter is NULL.\n");
		return -1;
	}

	pqti->ptraceInfo = NULL;
	return 0;
}

int init_trace_info(ptrace_info pti)
{
	if(!pti){
		return -1;
	}

	if(pti->sign_no){
		memset(pti->sign_no, 0x00, sizeof(pti->sign_no));
	}
	if(pti->incallid){
		memset(pti->incallid, 0x00, sizeof(pti->incallid));
	}
	if(pti->outcallid){
		memset(pti->outcallid, 0x00, sizeof(pti->outcallid));
	}
	return 0;
}

int db_disconnect_to_database(MYSQL* mysql)
{
  if(mysql){
    mysql_close(mysql);
    return 0;
  }

  syslog(LOG_ERR, "Disconnected mysql database. Mysql maybe null.\n");
  return -1;
}

int db_put_database_connection(MYSQL* mysql)
{
  return db_disconnect_to_database(mysql);
}

MYSQL*  db_connect_to_database(MYSQL* mysql, const char* host, const char* username, 
                                const char* password, const char* dbname)
{
  MYSQL* mysql_ptr = NULL;
  if(!dbname){
    syslog(LOG_ERR, "DBname error.\n");
    return NULL;
  }

  mysql_ptr = mysql_init(mysql);
  if(mysql_ptr == NULL){
    syslog(LOG_ERR, "Failed to initialize MySQL.\n");
    return NULL;
  }

  if(!mysql_real_connect(mysql_ptr, host, username, password, dbname, 0, NULL, 0)){
    syslog(LOG_ERR, "Failed to connect to database.\n");
    mysql_error(mysql_ptr);
    return NULL;
  }

  return mysql_ptr;
}

MYSQL* db_get_database_connection(MYSQL* mysql, const char* host, const char* username,
                                  const char* password, const char* dbname)
{
  return db_connect_to_database(mysql, host, username, password, dbname);
}

/* db interfaces, including create, delete, insert etc. */
int db_do_query_2(MYSQL* mysql, const char* query, callback cb, void* result)
{
  int affected_rows = 0;
  if(mysql_query(mysql, query) != 0){
    syslog(LOG_ERR, "Failed to query [%s], error:[%s]\n", query, mysql_error(mysql));
    return -1; 
  }

  if(mysql_field_count(mysql) > 0){
    MYSQL_RES* res = NULL;  // to store result set

    int num_fields = 0; // the number of the field
    int num_rows = 0;   // the number of the row

    if(!(res = mysql_store_result(mysql))){
      syslog(LOG_ERR, "Error [%s].\n", mysql_error(mysql));
      return -1;
    }

    num_fields = mysql_num_fields(res);
    num_rows = mysql_num_rows(res);

    if(cb){
      cb(result, res, num_fields, num_rows);
    }
		if(res != NULL)
			mysql_free_result(res);

    return num_rows;
  }else{
    affected_rows = mysql_affected_rows(mysql);
    syslog(LOG_DEBUG, " Operation %s affected rows %d.\n", query, affected_rows);
    return affected_rows;
  }

  return 0;
}

void callback_print_query_info(void* result, MYSQL_ROW row, int num_fields, int num_rows)
{
  int i;
  for(i = 0; i < num_fields; i++){
    printf("Fields[%d]: %s", i, row[i]);
  }
  printf("\n");
  return;
}

int callback_get_business_info(void *arg, MYSQL_RES* resultSet, int num_fields, int num_rows)
{
  pquery_business_info result = (pquery_business_info)arg;

  MYSQL_ROW row;
  if(result == NULL){
    printf("%s function %d line bad input parameter.\n", __func__, __LINE__);
    syslog(LOG_ERR, "%s function %d bad input parameter.\n", __func__, __LINE__);
    return -1;
  }

  printf("Num_fields:%d, num_rows:%d\n", num_fields, num_rows);
  if(result && (result->pbusinessInfo == NULL) && num_fields > 0 && num_rows > 0){
    result->pbusinessInfo = (pbusiness_info)malloc(num_rows * sizeof(business_info));
    if(NULL == result->pbusinessInfo){
      syslog(LOG_ERR, "%s function %d memory malloc failed.\n", __func__,__LINE__);
      return -1;
    }

    memset(result->pbusinessInfo, 0x00, num_rows * sizeof(business_info));
    result->num_columns = num_fields;
    result->num_rows = num_rows;
    result->cur_row = 0;
  }

  if(result->pbusinessInfo != NULL && result->cur_row < result->num_rows){
    while((row = mysql_fetch_row(resultSet)) != NULL){
      if(row[0]){
        strcpy(result->pbusinessInfo[result->cur_row].sign_number, row[0]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].sign_number, ""); 
      }

      if(row[1]){
        strcpy(result->pbusinessInfo[result->cur_row].called_format, row[1]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].called_format, ""); 
      }

      if(row[2]){
        strcpy(result->pbusinessInfo[result->cur_row].sign_type, row[2]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].sign_type, ""); 
      }

      if(row[3]){
        strcpy(result->pbusinessInfo[result->cur_row].record_flag, row[3]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].record_flag, ""); 
      }

      if(row[4]){
        strcpy(result->pbusinessInfo[result->cur_row].sign_code, row[4]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].sign_code, ""); 
      }

      if(row[5]){
        strcpy(result->pbusinessInfo[result->cur_row].custom_id, row[5]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].custom_id, ""); 
      }

      if(row[6]){
        strcpy(result->pbusinessInfo[result->cur_row].playback_sound_address, row[6]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].playback_sound_address, ""); 
      }

      if(row[7]){
        result->pbusinessInfo[result->cur_row].bridge_flag = atoi(row[7]); 
      }else{
        result->pbusinessInfo[result->cur_row].bridge_flag = 0; 
      }

      if(row[8]){
        result->pbusinessInfo[result->cur_row].valid_flag = atoi(row[8]); 
      }else{
        result->pbusinessInfo[result->cur_row].valid_flag = 0; 
      }

      if(row[9]){
        result->pbusinessInfo[result->cur_row].use_origin_call = atoi(row[9]); 
      }else{
        result->pbusinessInfo[result->cur_row].use_origin_call = 0;
      }

      if(row[10]){
        result->pbusinessInfo[result->cur_row].use_origin_called = atoi(row[10]); 
      }else{
        result->pbusinessInfo[result->cur_row].use_origin_called = 0;
      }

      if(row[11]){
        strcpy(result->pbusinessInfo[result->cur_row].call_no, row[11]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].call_no, "");
      }

      if(row[12]){
        strcpy(result->pbusinessInfo[result->cur_row].called_no, row[12]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].called_no, "");
      }

      if(row[13]){
        strcpy(result->pbusinessInfo[result->cur_row].display_no, row[13]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].display_no, "");
      }

      if(row[14]){
        strcpy(result->pbusinessInfo[result->cur_row].real_called_no, row[14]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].real_called_no, "");
      }

      if(row[15]){
        strcpy(result->pbusinessInfo[result->cur_row].route, row[15]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].route, "");
      }

      if(row[16]){
        strcpy(result->pbusinessInfo[result->cur_row].domain, row[16]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].domain, "");
      }

      if(row[17]){
        strcpy(result->pbusinessInfo[result->cur_row].create_time, row[17]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].create_time, "");
      }

      if(row[18]){
        strcpy(result->pbusinessInfo[result->cur_row].create_user, row[18]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].create_user, "");
      }

      if(row[19]){
        strcpy(result->pbusinessInfo[result->cur_row].modify_time, row[19]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].modify_time, "");
      }

      if(row[20]){
        strcpy(result->pbusinessInfo[result->cur_row].modify_user, row[20]); 
      }else{
        strcpy(result->pbusinessInfo[result->cur_row].modify_user, "");
      }

      if(row[21]){
        result->pbusinessInfo[result->cur_row].trace_flag = atoi(row[21]); 
      }else{
        result->pbusinessInfo[result->cur_row].trace_flag = 0; 
      }

      result->cur_row++;
    }
    return 0;
  }

  return  -1;
}

int callback_get_trace_info(void* arg, MYSQL_RES* resultSet, int num_fields, int num_rows)
{
  pquery_trace_info result = (pquery_trace_info)arg;

  MYSQL_ROW row;
  if(result == NULL){
    printf("Invalid parameters.\n");
    syslog(LOG_ERR, "%s function %d bad input parameter.\n", __func__, __LINE__);
    return -1;
  }

  if(result && (result->ptraceInfo == NULL) && num_fields > 0 && num_rows > 0){
    result->ptraceInfo = (ptrace_info)malloc(num_rows * sizeof(trace_info));
    if(NULL == result->ptraceInfo){
      syslog(LOG_ERR, "%s function %d memory malloc failed.\n", __func__, __LINE__);
      return -1;
    }

    memset(result->ptraceInfo, 0x00, num_rows * sizeof(trace_info));
    result->num_columns = num_fields;
    result->num_rows = num_rows;
    result->cur_row = 0;
  }

  if(result->ptraceInfo != NULL && result->cur_row < result->num_rows){
    while((row = mysql_fetch_row(resultSet)) != NULL){
      printf("%d\n", result->cur_row);
			if(row[0]){
				strcpy(result->ptraceInfo[result->cur_row].sign_no, row[0]);
			}else{
				strcpy(result->ptraceInfo[result->cur_row].sign_no, "");
			}

      if(row[1]){
        strcpy(result->ptraceInfo[result->cur_row].incallid, row[1]);
      }else{
        strcpy(result->ptraceInfo[result->cur_row].incallid, "");
      }

      if(row[2]){
        strcpy(result->ptraceInfo[result->cur_row].outcallid, row[2]);
      }else{
        strcpy(result->ptraceInfo[result->cur_row].outcallid, "");
      }
      result->cur_row++;
    }
    return 0;
  }
  return -1;
}


/* select interfaces */
int db_get_business_info(MYSQL* mysql, pquery_business_info query_info)
{
  if(!mysql){
    printf("mysql is NULL.\n");
    return -1;
  }

  char * prepStmt = "select * from %s;";
  char queryStmt[256] = {0};
  int ret = 0;

  sprintf(queryStmt, prepStmt, "business_msg");
	printf("sql: [%s]\n", queryStmt);

  ret = db_do_query_2(mysql, queryStmt, callback_get_business_info, query_info);

  if(ret > 0){
    return 0;
  }

  return -1;
}

int db_get_business_info_with_where(MYSQL* mysql, const char* where, pquery_business_info query_info)
{
  if(!mysql || !where){
    printf("Invalid parameters.\n");
    return -1;
  }

  char * prepStmt = "select * from business_msg %s;";
  char queryStmt[2048] = {0};
  int ret = 0;

  sprintf(queryStmt, prepStmt, where);
	printf("sql: [%s]\n", queryStmt);

  ret = db_do_query_2(mysql, queryStmt, callback_get_business_info, query_info);

  if(ret > 0){
    return 0;
  }

  return -1;
}

int db_get_trace_info(MYSQL* mysql, pquery_trace_info query_info)
{
  if(!mysql){
    printf("mysql is NULL.\n");
    return -1;
  }

  char* prepStmt = "select * from %s;";
  char  queryStmt[256] = {0};
  int   ret = 0;

  sprintf(queryStmt, prepStmt, "trace_info");
	printf("sql: [%s]\n", queryStmt);

  ret = db_do_query_2(mysql, queryStmt, callback_get_trace_info, query_info);

  if(ret > 0){
    return 0;
  }

  return -1;
}

int db_get_trace_info_with_where(MYSQL* mysql, const char* where, pquery_trace_info query_info)
{
  if(!mysql || !where){
    printf("Invalid parameters.\n");
    return -1;
  }

  char* prepStmt = "select * from trace_info %s;";
  char  queryStmt[2048] = {0};
  int   ret = 0;

  sprintf(queryStmt, prepStmt, where);
	printf("sql: [%s]\n", queryStmt);

  ret = db_do_query_2(mysql, queryStmt, callback_get_trace_info, query_info);

  if(ret > 0){
    return 0;
  }

  return -1;
}

/* add table interfaces */
int db_add_trace_info(MYSQL *mysql, ptrace_info traceinfo)
{
	char* prepStmt = "insert into %s values('%s','%s','%s');";
	char*	queryStmt = (char*)malloc(2048);
	int		ret = 0;

	if(!mysql || !traceinfo || !prepStmt){
		if(queryStmt){
			free(queryStmt);
		}
		return -1;
	}

	sprintf(queryStmt, prepStmt, "trace_info", traceinfo->sign_no, traceinfo->incallid, traceinfo->outcallid);
	ret = db_do_query_2(mysql, queryStmt, NULL, NULL);

	if(queryStmt){
		free(queryStmt);
		queryStmt = NULL;
	}

	if(ret > 0){
		return 0;
	}

  return -1;
}

/* delete table interfaces */
int db_delete_trace_info(MYSQL* mysql, ptrace_info traceinfo)
{
	if(!mysql || !traceinfo){
		return -1;
	}

	char*	prepStmt = "delete from %s where incallid = '%s' or outcallid = '%s';";
	char	queryStmt[2048] = {0};
	int		ret = 0;

	sprintf(queryStmt, prepStmt, "trace_info", traceinfo->incallid, traceinfo->outcallid);
	ret = db_do_query_2(mysql, queryStmt, NULL, NULL);

	if(ret > 0){
		return 0;
	}
  return -1;
}

int db_update_trace_info_with_where(MYSQL* mysql, const char* setstr, const char* where)
{
	if(!mysql || !setstr || !where){
		printf("Invalid parameters.\n");
		return -1;
	}

	char* prepStmt = "UPDATE trace_info SET %s WHERE %s;";
	char	queryStmt[2048] = {0};
	int		ret = 0;

	sprintf(queryStmt, prepStmt, setstr, where);
	ret = db_do_query_2(mysql, queryStmt, NULL, NULL);

	if(ret > 0){
		return 0;
	}

	return -1;
}


/****************************************************************************
 * Fetch flag from business_msg table, to decide if to trace the call.
 * Created by zhou at 2019
 ****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <regex.h>
#include "../../sr_module.h"
#include "../../mod_fix.h"
#include "../../mem/mem.h"
#include "../../pvar.h"
#include "tools.h"
#include "trace_config.h"
#include "credis.h"
#include "logger.h"

/**
 * Global variables
 */
const char*				g_logfile = NULL;
Traceconfig				traceconfig;
pthread_rwlock_t			rwlock;
int					flag_output_file = 0;
int					flag_output_redis = 0;

static char*				config_file = NULL; // configuration file

static int			mod_init(void);	/* Module initialization function */
//static void			global_var_init(void);/* Global variables initialization function */
static void			mod_destroy(void);	/* Module destroy function */
static int			trace_sip(struct sip_msg *_msg, const char *sipstr);

static int			prepare(void);	/* Reading configuration file to prepare log... */
static int			write_handle(const char* msg);
//static int			get_flag_from_table(const char* sign_no, const char* incallid, 
//																		const char* outcallid);

static char*		get_svalue(struct sip_msg *_msg, gparam_p str);
static int			fixup_param_func_trace_sip(void **param, int param_no);

static param_export_t trace_sip_params[] = {
	{"config_file", STR_PARAM, &config_file},
	{0,0,0}
};

static cmd_export_t cmds[] = {
	{"trace_sip", (cmd_function)trace_sip, 1, fixup_param_func_trace_sip, 0, REQUEST_ROUTE|ONREPLY_ROUTE},
	{0,0,0,0,0,0}	
};

struct module_exports exports = {
	"trace_sip",             /* module name */
	MOD_TYPE_DEFAULT,       /* class of this module */
	MODULE_VERSION,
	DEFAULT_DLFLAGS,        /* dlopen flags */
	0,                      /* load function */
	NULL,                   /* OpenSIPs module dependencies */
	cmds,                   /* exported functions */
	0,                      /* exported async functions */
	trace_sip_params,        /* module parameters */
	0,                      /* exported statistics */
	0,                      /* exported MI functions */
	0,                      /* exported pseudo-variables */
	0,                      /* exported transformations */
	0,                      /* extra processes */
	mod_init,               /* module initialization function */
	0,                      /* response function */
	mod_destroy,            /* destroy function */
	0,                      /* per-child init function */
};

char *get_svalue(struct sip_msg *_msg, gparam_p _str)
{
	char*	s = NULL;
	int		len = 0;
	str		s0;

	if(parse_headers(_msg, HDR_EOH_F, 0) == -1){
		LM_ERR("Error occurs while parsing message.\n");
		return NULL;
	}

	if(_str){
		if(fixup_get_svalue(_msg, _str, &s0) != 0){
			LM_ERR("Connot print the format.\n");
			return NULL;
		}
	}else{
		s0.len = 0;
		s0.s   = 0;
	}

	len = s0.len; // Not including null-termination
	s = (char *)pkg_malloc(len + 1);
	if(!s){
		LM_ERR("No pkg memory left.\n");
		return NULL;
	}

	memset(s, 0x00, len + 1);
	memcpy(s, s0.s, len);
	s[len] = '\0';
	LM_INFO("get_svalue s = [%s] with length [%d], type = [%d].\n", s, len, _str->type);

	return s;
}

int fixup_param_func_trace_sip(void **param, int param_no)
{
	if(param_no == 1){//} || param_no == 2 || param_no == 3 || param_no == 4){
		return fixup_sgp(param);
	}else{
		LM_ERR("Wrong number of parameters.\n");
		return E_UNSPEC;
	}
}

//void global_var_init(void)
//{
//	g_logfile = NULL;
//	flag_output_file = 0;
//	flag_output_redis = 0;
//	init_traceconfig(&traceconfig);
//}

int mod_init(void)
{
	int ret = 0;
	ret = prepare();
	if(ret != 0){
		LM_ERR("Failed to prepare...\n");
		return ret;
	}

	return ret;
}

void mod_destroy(void)
{
	// mysql 
	// file
	// redis
}

int prepare(void)
{
	if(!config_file){
		LM_ERR("Configfile is NULL.\n");
		return -1;
	}	

	int			ret = 0;
	int			output_dest = 0;
	size_t	len = 0;
	char*		logfile = NULL;

	ret = get_config(config_file, &traceconfig);
	if(ret != 0){
		LM_ERR("Failed to get config.\n");
		return -1;
	}

	// local file name
	output_dest = traceconfig.output_dest;
	flag_output_file = output_dest & 0x0001;
	flag_output_redis = output_dest >> 1;	

	// get logger filename
	len = strlen(traceconfig.fileconfig.dir) + strlen(traceconfig.fileconfig.basename) 
																						+ strlen(traceconfig.fileconfig.suffix);
	logfile = (char*)malloc(len + 1);
	if(!logfile){
		LM_ERR("Failed to malloc memory for logger file.\n");
		//goto Err;
		return -1;
	}

	memset(logfile, 0x00, len+1);
	strcat(logfile, traceconfig.fileconfig.dir);
	strcat(logfile, traceconfig.fileconfig.basename);
	strcat(logfile, traceconfig.fileconfig.suffix);
	logfile[len] = '\0';
	LM_ERR("logfile = [%s]\n", logfile);
	g_logfile = logfile;

	return 0;
}

int write_handle(const char* msg)
{
	int output_dest = traceconfig.output_dest;
	const char*	ch = traceconfig.redisconfig.channel;
	const char* host = traceconfig.redisconfig.host;
	int port = traceconfig.redisconfig.port;

	switch(output_dest){
		case 1:
			LM_INFO("Need to write to file.\n");
			ct_write_to_file(g_logfile, msg);
			break;
		case 2:
			LM_INFO("Need to write to redis.\n");
			if(!ch){
				LM_ERR("Redis channel is NULL.\n");
				break;
			}
			write_to_redis(host, port, ch, msg);
			break;
		case 3:
			LM_INFO("Need to write to both.\n");
			ct_write_to_file(g_logfile, msg);
			if(!ch){
				LM_ERR("Redis channel is NULL.\n");
				break;
			}
			write_to_redis(host, port, ch, msg);
			break;
	}
	return 0;
}

int trace_sip(struct sip_msg *_msg, const char *sipstr)
{
	if(!sipstr){
		LM_ERR("Invalid parameter.\n");
		return 1; // Just for module can running continuously.
	}

	//int			ret = 0;
	char*		_sipstr = get_svalue(_msg, (gparam_p)sipstr);
	
	// check trace_info
	//ret = get_flag_from_table(_sign_no, _incallid, _outcallid);	
	write_handle(_sipstr);
	
	if(_sipstr){
		pkg_free(_sipstr);
		_sipstr = NULL;
	}

	return 1;// Notes: return 0 will to terminate this module.
}

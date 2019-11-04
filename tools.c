//#include "../../sr_module.h"
#include "tools.h"

void release_pointer(void **p)
{
	if(*p){
		free(*p);
		*p = NULL;
	}
}

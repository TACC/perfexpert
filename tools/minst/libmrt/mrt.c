
#include "mrt.h"

enum { SUCCESS=0, ERR_PARAMS, ERR_MEM };

static int record(void* p)
{
	return SUCCESS;
}

__indigo_namespace_struct const __indigo = { record };

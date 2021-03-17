#ifndef AUXILIARY_H
#define AUXILIARY_H

#include <stdlib.h>
#include <wchar.h>

#include "cJSON.h"

#define RET_NULL(x) {\
	cJSON_Delete(x);\
	return NULL;\
}

#define NUM_2_OBJ(o, k, v) {\
	if (!cJSON_AddNumberToObject(o, k, v)) {\
		RET_NULL(obj);\
	}\
}

#define NUM_2_OBJ_CMP(o, k, p, a, b) {\
	if (!prev || a != b) {\
		if (!cJSON_AddNumberToObject(o, k, b)) {\
			RET_NULL(o);\
		}\
	}\
}

#define BOOL_2_OBJ(o, k, v) {\
	if (!cJSON_AddBoolToObject(o, k, v)) {\
		RET_NULL(o);\
	}\
}

#define BOOL_2_OBJ_CMP(obj, key, prev, a, b) {\
	if (!prev || a != b) {\
		if (!cJSON_AddBoolToObject(obj, key, b)) {\
			RET_NULL(obj);\
		}\
	}\
}

cJSON* addWstrToObject(cJSON*, char*, const wchar_t*);

#endif

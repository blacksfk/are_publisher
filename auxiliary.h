#ifndef AUXILIARY_H
#define AUXILIARY_H

#include <stdlib.h>
#include <wchar.h>

#include "cJSON.h"

#define RET_NULL(x) {\
	cJSON_Delete(x);\
	return NULL;\
}

#define NUM_2_OBJ(obj, key, prev, a, b) {\
	if (!prev || a != b) {\
		if (!cJSON_AddNumberToObject(obj, key, b)) {\
			RET_NULL(obj);\
		}\
	}\
}

#define BOOL_2_OBJ(obj, key, prev, a, b) {\
	if (!prev || a != b) {\
		if (!cJSON_AddBoolToObject(obj, key, b)) {\
			RET_NULL(obj);\
		}\
	}\
}

// epitome of laziness; request more than enough memory for the string conversion
#define MB_STR_LEN 256

cJSON* addWstrToObject(cJSON*, char*, wchar_t*);

#endif

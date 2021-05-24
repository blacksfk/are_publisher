#ifndef AUXILIARY_H
#define AUXILIARY_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wchar.h>

#include <cjson/cJSON.h>

/**
 * Delete (free) the provided cJSON object and return NULL.
 */
#define RET_NULL(x) do {\
	cJSON_Delete(x);\
	return NULL;\
} while(0)

/**
 * Add the number v to the object o under the key k. Deletes o and returns NULL
 * if adding the number failed.
 */
#define NUM_2_OBJ(o, k, v) do {\
	if (!cJSON_AddNumberToObject(o, k, v)) {\
		RET_NULL(o);\
	}\
} while(0)

/**
 * The same as NUM_2_OBJ but adds the number b to the object o under the key k if p is NULL
 * or a and b are not equal. Used to compare between previous and current data frames.
 * Deletes o and returns NULL if adding the number failed.
 * Example:
 * NUM_2_OBJ_CMP(obj, "position", prev, prev->position, curr->position)
 */
#define NUM_2_OBJ_CMP(o, k, p, a, b) do {\
	if (!prev || a != b) {\
		if (!cJSON_AddNumberToObject(o, k, b)) {\
			RET_NULL(o);\
		}\
	}\
} while(0)

/**
 * Add the boolean v to the object o under the key k. Deletes o and returns NULL
 * if adding the boolean failed.
 */
#define BOOL_2_OBJ(o, k, v) do {\
	if (!cJSON_AddBoolToObject(o, k, v)) {\
		RET_NULL(o);\
	}\
} while(0)

/**
 * The same as BOOL_2_OBJ but adds the boolean b to the object o under the key k if p is NULL
 * or a and b are not equal. Used to compare between previous and current data frames.
 * Deletes o and returns NULL if adding the boolean failed.
 * Example:
 * BOOL_2_OBJ_CMP(obj, "globalYellow", prev, prev->globalYellow, curr->globalYellow)
 */
#define BOOL_2_OBJ_CMP(o, k, p, a, b) do {\
	if (!p || a != b) {\
		if (!cJSON_AddBoolToObject(o, k, b)) {\
			RET_NULL(o);\
		}\
	}\
} while(0)

cJSON* addWstrToObject(cJSON*, char*, const wchar_t*);

#endif

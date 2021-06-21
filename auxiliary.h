#ifndef AUXILIARY_H
#define AUXILIARY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wchar.h>
#include <math.h>
#include <cjson/cJSON.h>

#include "error.h"

// 1kB should be enough for static error messages, right?
#define MSG_BOX_BUF_SIZE 1024

// prevent MSVC from spitting out warnings about "in-secure" functions
#pragma warning(disable:4996)

// prevent winsock 1.1 from being included by windows.h in order for libcurl to
// include winsock2.h
#define WIN32_LEAN_AND_MEAN

// classic MS and non-conforming code
// supress C5105 (undefined behaviour of "defined" in macro expansion) for the windows
// header files and then re-enable it for the rest of the program
#pragma warning(disable:5105)
#include <windows.h>
#pragma warning(default:5105)

/**
 * Delete (free) the provided cJSON object and return NULL.
 */
#define RET_NULL(x) do {\
	cJSON_Delete(x);\
	return NULL;\
} while(0)

/**
 * Add the integer v to the object o under the key k. Deletes o and returns NULL
 * if adding the integer failed.
 */
#define INT_2_OBJ(o, k, v) do {\
	if (!cJSON_AddNumberToObject(o, k, v)) {\
		RET_NULL(o);\
	}\
} while(0)

/**
 * The same as INT_2_OBJ but adds the integer b to the object o under the key k if p is NULL
 * or a and b are not equal. Used to compare between previous and current data frames.
 * Deletes o and returns NULL if adding the integer failed.
 * Example:
 * INT_2_OBJ_CMP(obj, "position", prev, prev->position, curr->position)
 */
#define INT_2_OBJ_CMP(o, k, p, a, b) do {\
	if (!prev || a != b) {\
		if (!cJSON_AddNumberToObject(o, k, b)) {\
			RET_NULL(o);\
		}\
	}\
} while(0)

/**
 * Similar to INT_2_OBJ but truncates v to three decimal places and adds it to the
 * object o under the key k. Deletes o and returns NULL if adding the float failed.
 * adding the float failed.
 */
#define FLOAT_2_OBJ(o, k, v) do {\
	float f = truncf(1000 * v) / 1000;\
	if (!cJSON_AddNumberToObject(o, k, f)) {\
		RET_NULL(o);\
	}\
} while (0)

/**
 * Same as FLOAT_2_OBJ but adds the float b to the object o under the key k if p
 * is NULL or a and b are not equal. Used to compare between previous and current
 * data frames. Deletes o and returns NULL if adding the float failed.
 */
#define FLOAT_2_OBJ_CMP(o, k, p, a, b) do {\
	if (!prev) {\
		float f = truncf(1000 * b) / 1000;\
		if (!cJSON_AddNumberToObject(o, k, f)) {\
			RET_NULL(o);\
		}\
	} else {\
		float f = truncf(1000 * a);\
		float g = truncf(1000 * b);\
		if (f != g) {\
			if (!cJSON_AddNumberToObject(o, k, g / 1000)) {\
				RET_NULL(o);\
			}\
		}\
	}\
} while (0)

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

char* wstrToStr(const wchar_t* wstr);
wchar_t* strToWstr(const char* str);
cJSON* addWstrToObject(cJSON*, char*, const wchar_t*);
void msgBoxErr(HWND parent, int e, const wchar_t* str);

#endif

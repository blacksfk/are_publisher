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
#include "config.h"

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
		INT_2_OBJ(o, k, b);\
	}\
} while(0)

// maximum number of bytes (including the decimal point) of a float represented
// as a string.
#define JSON_RAW_FLOAT_WIDTH 20

/**
 * Similar to INT_2_OBJ but rounds and truncates v to three decimal places and adds it to the
 * object o under the key k. Deletes o and returns NULL if adding the float failed.
 * adding the float failed.
 */
#define FLOAT_2_OBJ(o, k, v) do {\
	char raw[JSON_RAW_FLOAT_WIDTH];\
	snprintf(raw, JSON_RAW_FLOAT_WIDTH, "%.3f", v);\
	if (!cJSON_AddRawToObject(o, k, raw)) {\
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
		FLOAT_2_OBJ(o, k, b);\
	} else {\
		float f1 = truncf(a * 1000);\
		float f2 = truncf(b * 1000);\
		if (f1 != f2) {\
			FLOAT_2_OBJ(o, k, b);\
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
		BOOL_2_OBJ(o, k, b);\
	}\
} while(0)

char* wstrToStr(const wchar_t* wstr);
wchar_t* strToWstr(const char* str);
cJSON* addWstrToObject(cJSON*, char*, const wchar_t*);
void msgBoxErr(HWND parent, int e, const wchar_t* str);

#endif

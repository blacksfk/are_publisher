#include "auxiliary.h"

/**
 * Convert a wchar_t* to a char* and it to obj under key.
 *
 * @param  obj  The object to add the string to.
 * @param  key  The key under which the string will be stored.
 * @param  wstr The string to convert
 * @return      Value of cJSON_AddStringToObject()
 */
cJSON* addWstrToObject(cJSON* obj, char* key, wchar_t* wstr) {
	char mbstr[MB_STR_LEN + 1];
	size_t len = sizeof(mbstr);

	// wcstombs is deprecated according to microsoft...
	wcstombs_s(NULL, mbstr, len, wstr, len - 1);

	return cJSON_AddStringToObject(obj, key, mbstr);
}

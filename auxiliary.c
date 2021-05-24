#include "auxiliary.h"

/**
 * Convert a wchar_t* to a char* and it to obj under key.
 *
 * @param  obj  The object to add the string to.
 * @param  key  The key under which the string will be stored.
 * @param  wstr The string to convert
 * @return      Value of cJSON_AddStringToObject() or NULL if an error occurred.
 */
cJSON* addWstrToObject(cJSON* obj, char* key, const wchar_t* wstr) {
	// assume each wchar_t will occupy 2 bytes to guarantee wstr will fit in mbstr
	// add one for the terminating null
	size_t len = (wcslen(wstr) + 1) * 2;
	char* mbstr = malloc(len);

	if (!mbstr) {
		// allocation failed for some reason
		return NULL;
	}

	// insertion of null terminator ('\0') should have no issue due to calculation above,
	// but subtract one anyway to be safe
	if (wcstombs_s(NULL, mbstr, len, wstr, len - 1) != 0) {
		// conversion failed
		free(mbstr);

		return NULL;
	}

	// add the string to the object
	cJSON* ptr = cJSON_AddStringToObject(obj, key, mbstr);

	// free the dynamically allocated multi-byte string
	free(mbstr);

	return ptr;
}

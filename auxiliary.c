#include "auxiliary.h"

/**
 * Convert a wchar_t* to char*.
 * @param  wstr
 * @return      Allocated on the heap. Remember to free it.
 */
char* wstrToStr(const wchar_t* wstr) {
	// allocate sizeof(wchar_t) bytes for every character
	// in wstr and one extra for the null terminator
	size_t bytes = wcslen(wstr) * sizeof(wchar_t) + 1;
	char* str = malloc(bytes);

	if (!str) {
		// out of memory
		return NULL;
	}

	wcstombs(str, wstr, bytes);

	return str;
}

/**
 * Convert a char* to wchar_t*.
 * @param  str
 * @return     Allocated on the heap. Remember to free it.
 */
wchar_t* strToWstr(const char* str) {
	// allocate the length of str * sizeof(wchar_t) + 2 for wide null terminator
	size_t bytes = strlen(str) * sizeof(wchar_t) + 2;
	wchar_t* wstr = malloc(bytes);

	if (!wstr) {
		// out of memory
		return NULL;
	}

	mbstowcs(wstr, str, bytes);

	return wstr;
}

/**
 * Convert a wchar_t* to a char* and it to obj under key.
 * @param  obj  The object to add the string to.
 * @param  key  The key under which the string will be stored.
 * @param  wstr The string to convert
 * @return      Value of cJSON_AddStringToObject() or NULL if an error occurred.
 */
cJSON* addWstrToObject(cJSON* obj, char* key, const wchar_t* wstr) {
	char* mbstr = wstrToStr(wstr);

	if (!mbstr) {
		// out of memory
		return NULL;
	}

	// add the string to the object
	cJSON* ptr = cJSON_AddStringToObject(obj, key, mbstr);

	// free the dynamically allocated multi-byte string
	free(mbstr);

	return ptr;
}

/**
 * Show a message box with an error code and static message.
 * @param e   The error code.
 * @param str The error message.
 */
void msgBoxErr(int e, const wchar_t* str) {
	wchar_t msg[MSG_BOX_BUF_SIZE];

	swprintf(msg, MSG_BOX_BUF_SIZE, L"%d: %ls.", e, str);
	MessageBoxW(NULL, msg, L"Error", MB_OK);
}

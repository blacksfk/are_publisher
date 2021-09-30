#ifndef ERROR_H
#define ERROR_H

#include <stddef.h>

enum areError {
	ARE_SHARED_MEM_INIT = 32000,
	ARE_OUT_OF_MEM,
	ARE_CURL,
	ARE_REQ,
	ARE_REQ_TIMEOUT,
	ARE_SERVER,
	ARE_GUI,
	ARE_USER_INPUT,
	ARE_THREAD,
	ARE_FILE,
	ARE_EVENT
};

wchar_t* errorToWstr(enum areError);

#endif

#include "error.h"

/**
 * Get the corresponding error message for the error code.
 * @param  e
 * @return   String literal
 */
wchar_t* errorToWstr(enum areError e) {
	switch (e) {
	case ARE_SHARED_MEM_INIT:
		return L"Shared memory initialisation failed";
	case ARE_OUT_OF_MEM:
		return L"Out of memory";
	case ARE_CURL:
		return L"Libcurl error";
	case ARE_REQ:
		return L"Request error";
	case ARE_REQ_TIMEOUT:
		return L"Request timed out";
	case ARE_GUI:
		return L"GUI error";
	case ARE_USER_INPUT:
		return L"Invalid input";
	case ARE_THREAD:
		return L"Processing thread error";
	case ARE_FILE:
		return L"Log file error";
	case ARE_EVENT:
		return L"Thread event error";
	}

	return L"Unknown";
}

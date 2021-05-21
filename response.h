#ifndef RESPONSE_H
#define RESPONSE_H

#pragma warning(disable:4996)

#include <stdlib.h>
#include <string.h>
// classic MS and non-conforming code
// supress C5105 (undefined behaviour of "defined" in macro expansion) for the windows
// header files (included by libcurl)
#pragma warning(disable:5105)
#include <curl/curl.h>

#define BUF_SIZE 1024
#define HEADER_COUNT 10

struct payload {
	char* data;
	size_t len;
	size_t cap;
};

typedef struct header {
	char* key;
	char* value;
} Header;

typedef struct response {
	struct payload* body;
	CURLcode curlCode;
	int status;
	int headerCount;
	int headerCap;
	Header** headers;
} Response;

Response* createResponse();
Header* addResponseHeader(Response*, const char*, const char*);
void populateResponse(CURL*, Response*);
void freeResponse(Response* r);

#endif

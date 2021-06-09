#ifndef RESPONSE_H
#define RESPONSE_H

#include <stdlib.h>
#include <string.h>

// curl includes the windows headers so need to disable the warning here too
#pragma warning(disable:5105)
#include <curl/curl.h>
#pragma warning(default:5105)

// stop whinging about POSIX-standard function names
#pragma warning(disable:4996)

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

#ifndef RESPONSE_H
#define RESPONSE_H

#include <stdlib.h>
// classic MS and non-conforming code
// supress C5105 (undefined behaviour of "defined" in macro expansion) for the windows
// header files (included by libcurl)
#pragma warning(disable:5105)
#include <curl/curl.h>

#define BUF_SIZE 4096

struct payload {
	char* data;
	size_t len;
	size_t cap;
};

typedef struct response {
	struct payload* head;
	struct payload* body;
	CURLcode code;
} Response;

Response* createResponse();
void freeResponse(Response* r);

#endif

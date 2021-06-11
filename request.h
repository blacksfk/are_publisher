#ifndef HTTP_H
#define HTTP_H

#include <stdarg.h>
#include <stdbool.h>
#include "response.h"

#define HEADER_DELIMS " :\r\n"
#define HEADER_CHAN_PW "Channel-Password: "

#define CHAN_ENDPOINT "/channel"
#define PUB_ENDPOINT "publish/"

Response* publishData(CURL* curl, const char* url, const char* pwHeader, const char* json);
char* createPublishURL(const char* base, const char* cID);
char* createPasswordHeader(const char* password);

#endif

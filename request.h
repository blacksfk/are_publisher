#ifndef HTTP_H
#define HTTP_H

#include <stdarg.h>
#include <stdbool.h>
#include "response.h"

#define HEADER_DELIMS " :\r\n"
#define HEADER_CHAN_PW "Channel-Password: "

#define CHAN_ENDPOINT "/channel"
#define PUB_ENDPOINT "/publish"

char* createPasswordHeader(const char* password);
struct curl_slist* publishInit(CURL* curl, const char* base, const char* cID, const char* pw);
Response* publish(CURL* curl, const char* json);
Response* getChannels(CURL* curl, const char* base);

#endif

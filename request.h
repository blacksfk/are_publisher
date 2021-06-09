#ifndef HTTP_H
#define HTTP_H

#include <stdarg.h>
#include "response.h"

#define HEADER_DELIMS " :\r\n"
#define HEADER_CHAN_PW "Channel-Password: "

#define CHAN_ENDPOINT "/channel"
#define PUB_ENDPOINT "/publish/"

Response* getChannels(CURL* curl, const char* baseURL);
Response* publishData(CURL* curl, const char* baseURL,
						const char* cID, const char* cPW, const char* json);

#endif

#ifndef API_H
#define API_H

#include <stdarg.h>
#include <stdbool.h>
#include <cjson/cJSON.h>

#include "error.h"
#include "config.h"
#include "request.h"

#define REQ_TIMEOUT 5L
#define HEADER_CHAN_PW "Channel-Password: "
#define CHAN_ENDPOINT "/channel"
#define PUB_ENDPOINT "/publish"

// initial number of bytes to allocate
#define INIT_URL_STR_LEN 64

char* createPasswordHeader(const char* password);
struct curl_slist* publishInit(CURL* curl, const char* base, const char* cID, const char* pw);
int publish(CURL* curl, const char* json);
int getChannels(cJSON** ptr);
int channelLogin(char*, char*);

#endif

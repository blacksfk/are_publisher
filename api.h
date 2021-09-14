#ifndef API_H
#define API_H

#include <cjson/cJSON.h>

#include "error.h"
#include "config.h"
#include "request.h"

#define REQ_TIMEOUT 5L

int getChannelsJSON(cJSON** ptr);

#endif

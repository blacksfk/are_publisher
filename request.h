#ifndef HTTP_H
#define HTTP_H

#include "response.h"

#define HEADER_DELIMS " :\r\n"

Response* performRequest(CURL*);

#endif

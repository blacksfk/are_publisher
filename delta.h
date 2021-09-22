#ifndef DELTA_H
#define DELTA_H

#include "tracked.h"
#include "shared_mem.h"

#define JSON_BUF_SIZE 2048

char* deltaJSON(SharedMem*, Tracked*, bool);

#endif

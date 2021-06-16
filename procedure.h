#ifndef PROCEDURE_H
#define PROCEDURE_H

#include "request.h"
#include "instance_data.h"

#define JSON_BUF_SIZE 2048
#define SLEEP_DURATION 1000
#define MAX_LOOP_TIME 1000

DWORD WINAPI procedure(void* arg);

#endif

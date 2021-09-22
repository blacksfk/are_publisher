#ifndef PROCEDURE_H
#define PROCEDURE_H

#include "delta.h"
#include "request.h"
#include "instance_data.h"

#define SLEEP_DURATION 1000
#define MAX_LOOP_TIME 1000

DWORD WINAPI procedure(void* arg);

#endif

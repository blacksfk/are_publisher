#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <signal.h>

// classic MS and non-conforming code
// supress C5105 (undefined behaviour of "defined" in macro expansion) for the windows
// header files and then re-enable it for the rest of the program
#pragma warning(disable:5105)
#include <windows.h>
#pragma warning(default:5105)

#include "physics.h"
#include "hud.h"
#include "properties.h"
#include "request.h"

#define SM_PHYSICS L"Local\\acpmf_physics"
#define SM_HUD L"Local\\acpmf_graphics"
#define SM_PROPS L"Local\\acpmf_static"

// program name, address, id, password
#define MIN_ARGS 4
#define JSON_BUF_SIZE 2048
#define WAIT_FOR_LIVE 5000
#define MAX_LOOP_TIME 1000

enum areError {
	ARE_INVALID_ARGS = 1000,
	ARE_SHARED_MEM_INIT,
	ARE_OUT_OF_MEM,
	ARE_CURL,
	ARE_REQ,
	ARE_DURATION
};

struct config {
	char* address;
	char* id;
	char* pw;
};

struct memMaps {
	HUD* hud;
	Physics* physics;
	Properties* props;
};

bool args(int, char**, struct config*);
LPVOID mapSharedMemory(wchar_t*, size_t);
CURL* initCurl();
char* toJSON(struct memMaps prev, struct memMaps curr, bool sendProps);
bool publish(CURL*, struct config, char*);
void* clone(const void*, size_t);

#endif

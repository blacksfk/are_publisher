#ifndef SHARED_MEM_H
#define SHARED_MEM_H

#include <stdlib.h>

// prevent winsock 1.1 from being included by windows.h in order for libcurl to
// include winsock2.h
#define WIN32_LEAN_AND_MEAN

// classic MS and non-conforming code
// supress C5105 (undefined behaviour of "defined" in macro expansion) for the windows
// header files and then re-enable it for the rest of the program
#pragma warning(disable:5105)
#include <windows.h>
#pragma warning(default:5105)

#include "physics.h"
#include "hud.h"
#include "properties.h"

#define SM_PHYSICS L"Local\\acpmf_physics"
#define SM_HUD L"Local\\acpmf_graphics"
#define SM_PROPS L"Local\\acpmf_static"

struct memMaps {
	HUD* hud;
	Physics* physics;
	Properties* props;
};

typedef struct sharedMem {
	struct memMaps curr;
	struct memMaps prev;
	size_t szHud;
	size_t szPhysics;
	size_t szProps;
} SharedMem;

SharedMem* createSharedMem();
void freeSharedMem(SharedMem* sm);
void sharedMemCurrToPrev(SharedMem* sm);

#endif

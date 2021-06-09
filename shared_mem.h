#ifndef SHARED_MEM_H
#define SHARED_MEM_H

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

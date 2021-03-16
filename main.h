#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <windows.h>

#include "physics.h"
#include "hud.h"
#include "properties.h"

#define SM_PHYSICS L"Local\\acpmf_physics"
#define SM_HUD L"Local\\acpmf_graphics"
#define SM_PROPS L"Local\\acpmf_static"

LPVOID mapSharedMemory(wchar_t*, size_t);

#endif

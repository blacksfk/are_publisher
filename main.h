#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

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

LPVOID mapSharedMemory(wchar_t*, size_t);

#endif

#ifndef GUI_H
#define GUI_H

// enable visual styles
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "controls.h"

#define WINDOW_CLASS L"are_publisher_class"
#define WINDOW_TITLE L"ACC Race Engineer"

void gui(HINSTANCE h, int cmdShow, InstanceData* data);

#endif

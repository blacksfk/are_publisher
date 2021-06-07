#ifndef GUI_H
#define GUI_H

// enable visual styles
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <stdlib.h>
#include <stdbool.h>

// prevent winsock 1.1 from being included by windows.h in order for libcurl to
// include winsock2.h
#define WIN32_LEAN_AND_MEAN

// classic MS and non-conforming code
// supress C5105 (undefined behaviour of "defined" in macro expansion) for the windows
// header files and then re-enable it for the rest of the program
#pragma warning(disable:5105)
#include <windows.h>
#pragma warning(default:5105)

#include "request.h"

enum buttons {
	BTN_TOGGLE = 1000
};

// struct that groups the window handlers of the form controls
struct formHandlers {
	HWND lblAddress;
	HWND lblChannel;
	HWND lblPassword;

	HWND ctrlAddress;
	HWND ctrlChannel;
	HWND ctrlPassword;

	HWND btnToggle;
};

typedef struct instanceData {
	wchar_t* address;
	wchar_t* channel;
	wchar_t* password;

	struct formHandlers handlers;
	CURL* curl;

	// TODO: status label, progress bar??
} InstanceData;

#define WINDOW_CLASS L"are_publisher_class"
#define WINDOW_TITLE L"ACC Race Engineer Publisher"

// main window width and height
#define WINDOW_W 500
#define WINDOW_H 300

// window internal margins
#define MARGIN_X 10
#define MARGIN_Y MARGIN_X

// form control parameters
#define FORM_CTRL_W (WINDOW_W - (MARGIN_X * 4))
#define FORM_CTRL_H 20

// form label parameters
#define FORM_LBL_W FORM_CTRL_W
#define FORM_LBL_H 15

// form button parameters
#define FORM_BTN_W FORM_CTRL_W
#define FORM_BTN_H FORM_CTRL_H

// gap between form elements
#define FORM_MARGIN_H 2

// the height of one label and control
#define FORM_GROUP_H (FORM_LBL_H + FORM_MARGIN_H + FORM_CTRL_H + FORM_MARGIN_H)

// styling
#define FORM_LBL_STYLE (WS_VISIBLE | WS_CHILD)
#define FORM_CTRL_STYLE (FORM_LBL_STYLE | WS_BORDER | ES_AUTOHSCROLL)
#define FORM_BTN_STYLE (FORM_LBL_STYLE | BS_CENTER)
#define FORM_BTN_HIDDEN_STYLE (WS_CHILD | BS_CENTER)

bool gui(HINSTANCE h, int cmdShow);

#endif

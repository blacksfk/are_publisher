#ifndef GUI_H
#define GUI_H

// enable visual styles
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <stdbool.h>

#include "shared_mem.h"
#include "request.h"

enum buttons {
	BTN_TOGGLE = 1000
};

// struct that groups the window handlers of the form controls
struct formHandlers {
	HWND lblAddress;
	HWND lblChannel;
	HWND lblPassword;
	HWND lblStatus;

	HWND ctrlAddress;
	HWND ctrlChannel;
	HWND ctrlPassword;

	HWND btnToggle;
};

typedef struct instanceData {
	// text input buffers
	wchar_t* address;
	wchar_t* channel;
	wchar_t* password;

	// curl easy handler
	CURL* curl;

	// shared memory object
	SharedMem* sm;

	// as described above
	struct formHandlers handlers;

	// function to run once WM_DESTROY is received
	// should free allocated resources etc.
	// must call freeInstanceData once other cleanup tasks are complete
	void (*cleanup)(struct instanceData*);
} InstanceData;

// how much text could you really enter?? 4096 wchar_t characters should be plenty...
#define FORM_CTRL_BUF_SIZE 8192

#define WINDOW_CLASS L"are_publisher_class"
#define WINDOW_TITLE L"ACC Race Engineer"

// main window width and height
#define WINDOW_W 500
#define WINDOW_H 220

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
#define FORM_LBL_STYLE (WS_VISIBLE | WS_CHILD | SS_SIMPLE)
#define FORM_CTRL_STYLE (WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL)
#define FORM_BTN_STYLE (WS_VISIBLE | WS_CHILD | BS_CENTER)
#define FORM_BTN_HIDDEN_STYLE (WS_CHILD | BS_CENTER)

InstanceData* createInstanceData(
	CURL* curl, SharedMem* sm, void (*cleanup)(struct instanceData*)
);
void freeInstanceData(InstanceData* data);
bool gui(HINSTANCE h, int cmdShow, InstanceData* data);

#endif

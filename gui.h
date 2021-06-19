#ifndef GUI_H
#define GUI_H

// enable visual styles
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "procedure.h"

enum buttons {
	BTN_TOGGLE = 1000
};

#define WINDOW_CLASS L"are_publisher_class"
#define WINDOW_TITLE L"ACC Race Engineer"

#define BTN_START L"Start"
#define BTN_STOP L"Stop"

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

void gui(HINSTANCE h, int cmdShow, InstanceData* data);

#endif

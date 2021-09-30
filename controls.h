#ifndef CONTROLS_H
#define CONTROLS_H

#include "procedure.h"

enum buttons {
	BTN_TOGGLE = 1000,
	BTN_REFRESH,
	BTN_INFO
};

#define BTN_START L"Start"
#define BTN_STOP L"Stop"

// main window width and height
#define WINDOW_W 500
#define WINDOW_H 175

// window internal margins
#define MARGIN_X 10
#define MARGIN_Y MARGIN_X

// form control parameters
#define FORM_CTRL_W (WINDOW_W - 100 - (MARGIN_X * 4))
#define FORM_CTRL_H 20

// form label parameters
#define FORM_LBL_W FORM_CTRL_W
#define FORM_LBL_H 15

// form button parameters
#define FORM_BTN_W FORM_CTRL_W
#define FORM_BTN_H (FORM_CTRL_H + 2)
#define FORM_BTN_I_W (WINDOW_W - 400 - (MARGIN_X * 4))
#define FORM_BTN_I_MARGIN_X (FORM_CTRL_W + (MARGIN_X * 2))

// gap between form elements
#define FORM_MARGIN_H 5

// the height of one label and control
#define FORM_GROUP_H (FORM_LBL_H + FORM_MARGIN_H + FORM_CTRL_H + FORM_MARGIN_H)

// styling
#define FORM_LBL_STYLE (WS_VISIBLE | WS_CHILD | SS_SIMPLE)
#define FORM_BTN_STYLE (WS_VISIBLE | WS_CHILD | BS_CENTER | WS_GROUP | WS_TABSTOP)
#define FORM_CTRL_STYLE (WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | WS_GROUP | WS_TABSTOP)

int getHandlerText(InstanceData*);
bool createControls(HWND, struct formHandlers*);

#endif

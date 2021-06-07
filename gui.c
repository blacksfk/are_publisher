#include "gui.h"

/**
 * Free all heap allocated members of an instance data struct.
 */
static void freeInstanceData(InstanceData* data) {
	free(data->address);
	free(data->channel);
	free(data->password);
}

/**
 * Create the static and edit controls and bind them to the parent window.
 * @param parent
 * @param handlers
 */
static void createForm(HWND parent, struct formHandlers* handlers) {
	// server address label
	handlers->lblAddress = CreateWindowW(
		L"Static",
		L"Server address",
		FORM_LBL_STYLE,
		MARGIN_X,
		MARGIN_Y,
		FORM_LBL_W,
		FORM_LBL_H,
		parent,
		NULL,
		NULL,
		NULL
	);

	// server address input
	handlers->ctrlAddress = CreateWindowW(
		L"Edit",
		NULL,
		FORM_CTRL_STYLE,
		MARGIN_X,
		MARGIN_Y + FORM_LBL_H + FORM_MARGIN_H,
		FORM_CTRL_W,
		FORM_CTRL_H,
		parent,
		NULL,
		NULL,
		NULL
	);

	// channel ID label
	handlers->lblChannel = CreateWindowW(
		L"Static",
		L"Channel",
		FORM_LBL_STYLE,
		MARGIN_X,
		MARGIN_Y + FORM_GROUP_H,
		FORM_LBL_W,
		FORM_LBL_H,
		parent,
		NULL,
		NULL,
		NULL
	);

	// channel ID input
	handlers->ctrlChannel = CreateWindowW(
		L"Edit",
		NULL,
		FORM_CTRL_STYLE,
		MARGIN_X,
		MARGIN_Y + FORM_GROUP_H + FORM_LBL_H + FORM_MARGIN_H,
		FORM_CTRL_W,
		FORM_CTRL_H,
		parent,
		NULL,
		NULL,
		NULL
	);

	// channel password label
	handlers->lblPassword = CreateWindowW(
		L"Static",
		L"Channel Password",
		FORM_LBL_STYLE,
		MARGIN_X,
		MARGIN_Y + FORM_GROUP_H * 2,
		FORM_LBL_W,
		FORM_LBL_H,
		parent,
		NULL,
		NULL,
		NULL
	);

	// channel password input
	handlers->ctrlPassword = CreateWindowW(
		L"Edit",
		NULL,
		FORM_CTRL_STYLE | ES_PASSWORD,
		MARGIN_X,
		MARGIN_Y + (FORM_GROUP_H * 2) + FORM_LBL_H + FORM_MARGIN_H,
		FORM_CTRL_W,
		FORM_CTRL_H,
		parent,
		NULL,
		NULL,
		NULL
	);

	// button
	handlers->btnToggle = CreateWindowW(
		L"Button",
		L"Start",
		FORM_BTN_STYLE,
		MARGIN_X,
		MARGIN_Y + FORM_GROUP_H * 3,
		FORM_BTN_W,
		FORM_BTN_H,
		parent,
		(HMENU) BTN_TOGGLE,
		NULL,
		NULL
	);
}

/**
 * WindowProc callback.
 * @param  wnd
 * @param  msg
 * @param  w
 * @param  l
 */
static LRESULT CALLBACK windowProcess(HWND wnd, UINT msg, WPARAM w, LPARAM l) {
	InstanceData* data = NULL;

	switch (msg) {
	case WM_CREATE:
		// extract the form field handler struct from the pointer
		CREATESTRUCT* cs = (CREATESTRUCT*) l;
		data = (InstanceData*) cs->lpCreateParams;

		// embed the struct as instance data
		SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR) data);

		return 0;
	case WM_PAINT:
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(wnd, &ps);

		FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));
		EndPaint(wnd, &ps);

		return 0;
	case WM_COMMAND:
		// TODO
		return 0;
	case WM_DESTROY:
		freeInstanceData(data);
		PostQuitMessage(0);

		return 0;
	}

	return DefWindowProc(wnd, msg, w, l);
}

/**
 * EnumChildWindows callback. Sends WM_SETFONT for each child window recursively.
 * @param  wnd
 * @param  l
 */
static BOOL CALLBACK enumChildProcesses(HWND wnd, LPARAM l) {
	HFONT font = (HFONT) l;

	SendMessage(wnd, WM_SETFONT, (WPARAM) font, MAKELPARAM(true, 0));

	return true;
}

/**
 * Obtains the system default font and sets it for the window passed in along with
 * all of it's children.
 * @param wnd
 */
static void setFont(HWND wnd) {
	NONCLIENTMETRICS metrics;

	metrics.cbSize = sizeof(metrics);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, metrics.cbSize, &metrics, 0);

	// create a font handler
	HFONT font = CreateFontIndirect(&metrics.lfCaptionFont);

	// propagate the font to the window and all child windows
	EnumChildWindows(wnd, &enumChildProcesses, (LPARAM) font);
}

/**
 * Initialise, create, and dispatch messages to the main window.
 * @param  h
 * @param  cmdShow
 * @return         Returns false if the window could not be created and true otherwise.
 */
bool gui(HINSTANCE h, int cmdShow) {
	InstanceData data;
	const wchar_t* class = WINDOW_CLASS;
	WNDCLASSW wc = {
		.lpfnWndProc = &windowProcess,
		.hInstance = h,
		.lpszClassName = class
	};

	RegisterClassW(&wc);

	// create main window
	HWND wnd = CreateWindowW(
		class,
		WINDOW_TITLE,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		WINDOW_W,
		WINDOW_H,
		NULL,
		NULL,
		h,
		&data
	);

	if (!wnd) {
		return false;
	}

	// create and add the form controls to the main window
	createForm(wnd, &data.handlers);

	// show the window
	ShowWindow(wnd, cmdShow);

	// set the font of the main window and its children to the default system font
	setFont(wnd);

	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

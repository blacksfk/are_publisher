#include "gui.h"

/**
 * Copies the input from window handlers into their respective buffers.
 * @param  data
 * @return      True if all buffers contain at least one byte and false otherwise.
 */
static bool getHandlerText(InstanceData* data) {
	// copy the input from each handle into the respective buffer
	// GetWindowTextW will truncate and append the the wide null terminator
	// if the input byte count is > FORM_CTRL_BUF_SIZE - 2
	int a = GetWindowTextW(
		data->handlers.ctrlAddress,
		data->address,
		FORM_CTRL_BUF_SIZE
	);

	int c = GetWindowTextW(
		data->handlers.ctrlChannel,
		data->address,
		FORM_CTRL_BUF_SIZE
	);

	int p = GetWindowTextW(
		data->handlers.ctrlPassword,
		data->password,
		FORM_CTRL_BUF_SIZE
	);

	// GetWindowTextW returns the number of characters copied
	return (a && c && p);
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
		BTN_START,
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

	// status label
	handlers->lblStatus = CreateWindowW(
		L"Static",
		L"Status: ",
		FORM_LBL_STYLE,
		MARGIN_X,
		MARGIN_Y + (FORM_GROUP_H * 3) + FORM_BTN_H + FORM_MARGIN_H,
		FORM_LBL_W,
		FORM_LBL_H,
		parent,
		NULL,
		NULL,
		NULL
	);
}

/**
 * Handles WM_CREATE.
 */
static LRESULT wmCreate(HWND wnd, UINT msg, WPARAM w, LPARAM l) {
	(void) msg;
	(void) w;

	// extract the instance data from the pointer
	CREATESTRUCT* cs = (CREATESTRUCT*) l;
	InstanceData* data = (InstanceData*) cs->lpCreateParams;

	// embed the struct in the window as user data
	SetWindowLongPtr(wnd, GWLP_USERDATA, (LONG_PTR) data);

	return 0;
}

/**
 * Handles WM_PAINT.
 */
static LRESULT wmPaint(HWND wnd, UINT msg, WPARAM w, LPARAM l) {
	(void) msg;
	(void) w;
	(void) l;

	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(wnd, &ps);

	FillRect(hdc, &ps.rcPaint, (HBRUSH) COLOR_WINDOW + 1);
	EndPaint(wnd, &ps);

	return 0;
}

/**
 * Handles WM_COMMAND.
 */
static LRESULT wmCommand(HWND wnd, UINT msg, WPARAM w, LPARAM l) {
	if (w != BTN_TOGGLE) {
		// something else was clicked; let windows handle it
		return DefWindowProcW(wnd, msg, w, l);
	}

	// extract the struct from the window user data
	InstanceData* data = (InstanceData*) GetWindowLongPtr(wnd, GWLP_USERDATA);
	HWND btn = data->handlers.btnToggle;

	// disable the button until the toggle operation has completed
	EnableWindow(btn, false);

	// toggle the state of the program
	if (!data->running) {
		// copy text into input buffers
		if (!getHandlerText(data)) {
			EnableWindow(btn, true);
			msgBoxErr(ARE_USER_INPUT, L"Invalid input");

			return 0;
		}

		// TODO: validate input

		// create a thread
		data->thread = CreateThread(
			NULL,           // default security attributes
			0,              // default stack size
			&procedure,     // the function
			data,           // function args
			0,              // start the thread immediately
			&data->threadId // thread id
		);

		if (!data->thread) {
			EnableWindow(btn, true);
			msgBoxErr(ARE_THREAD, L"Failed to create thread");

			return 0;
		}

		// change the text of the button to "Stop"
		SetWindowTextW(btn, BTN_STOP);
	} else {
		// stop the running thread by sending it a WM_QUIT message
		PostThreadMessageW(data->threadId, WM_QUIT, 0, 0);

		// wait until the thread terminates
		// TODO: handle thread not terminating after a period of time
		WaitForSingleObject(data->thread, INFINITE);

		// change the text of the button to "Start"
		SetWindowTextW(btn, BTN_START);
	}

	// re-enable the button and toggle the state
	EnableWindow(btn, true);
	data->running = !data->running;

	return 0;
}

/**
 * Handles WM_DESTROY.
 */
static LRESULT wmDestroy(HWND wnd, UINT msg, WPARAM w, LPARAM l) {
	(void) msg;
	(void) w;
	(void) l;

	// extract struct from the window user data
	InstanceData* data = (InstanceData*) GetWindowLongPtr(wnd, GWLP_USERDATA);

	// run the cleanup function to release held resources
	data->cleanup(data);

	// signal the loop to exit
	PostQuitMessage(0);

	return 0;
}

/**
 * WindowProc callback.
 */
static LRESULT CALLBACK windowProcess(HWND wnd, UINT msg, WPARAM w, LPARAM l) {
	switch (msg) {
	case WM_CREATE:
		return wmCreate(wnd, msg, w, l);
	case WM_PAINT:
		return wmPaint(wnd, msg, w, l);
	case WM_COMMAND:
		return wmCommand(wnd, msg, w, l);
	case WM_DESTROY:
		return wmDestroy(wnd, msg, w, l);
	}

	return DefWindowProcW(wnd, msg, w, l);
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
bool gui(HINSTANCE h, int cmdShow, InstanceData* data) {
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
		data
	);

	if (!wnd) {
		return false;
	}

	// create and add the form controls to the main window
	createForm(wnd, &data->handlers);

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

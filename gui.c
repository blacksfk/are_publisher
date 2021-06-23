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
		data->channel,
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
 * @param  parent   Used as hWndParent argument to CreateWindow.
 * @param  handlers Members are set to their respective window handlers.
 * @return          True if all windows were created and false otherwise.
 */
static bool createControls(HWND parent, struct formHandlers* handlers) {
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

	// return true if all of the windows were created successfully
	// and false otherwise
	return (
		handlers->lblAddress &&
		handlers->lblChannel &&
		handlers->lblPassword &&
		handlers->ctrlAddress &&
		handlers->ctrlChannel &&
		handlers->ctrlPassword &&
		handlers->btnToggle
	);
}

/**
 * Send a terminate message to the thread and wait for it to exit.
 * @param thread
 * @param threadId
 */
static void terminateThread(HANDLE thread, DWORD threadId) {
	// stop the running thread by sending it a WM_QUIT message
	PostThreadMessageW(threadId, WM_QUIT, 0, 0);

	// wait until the thread terminates
	// TODO: handle thread not terminating after a period of time
	WaitForSingleObject(thread, INFINITE);
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

	FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW + 1));
	EndPaint(wnd, &ps);

	return 0;
}

/**
 * Handles WM_CTLCOLORSTATIC. Sets the background of static controls to be
 * transparent.
 */
static LRESULT wmCtlColorStatic(HWND wnd, UINT msg, WPARAM w, LPARAM l) {
	(void) wnd;
	(void) msg;
	(void) l;

	HDC ctrl = (HDC) w;

	// set the background colour to transparent
	SetBkMode(ctrl, TRANSPARENT);

	return (LRESULT) GetSysColorBrush(TRANSPARENT);
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
			msgBoxErr(NULL, ARE_USER_INPUT, L"All fields are required");

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
			msgBoxErr(wnd, ARE_THREAD, L"Failed to create thread");

			return 0;
		}

		// change the text of the button to "Stop"
		SetWindowTextW(btn, BTN_STOP);
	} else {
		terminateThread(data->thread, data->threadId);

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

	if (data->running) {
		// thread is running so kill it
		terminateThread(data->thread, data->threadId);
	}

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
	case WM_CTLCOLORSTATIC:
		return wmCtlColorStatic(wnd, msg, w, l);
	case WM_COMMAND:
		return wmCommand(wnd, msg, w, l);
	case WM_DESTROY:
		return wmDestroy(wnd, msg, w, l);
	}

	return DefWindowProcW(wnd, msg, w, l);
}

/**
 * Register and create the window.
 * @param  h
 * @param  data
 * @return      NULL if CreateWindowW failed.
 */
static HWND init(HINSTANCE h, InstanceData* data) {
	WNDCLASSW wc = {
		.lpfnWndProc = &windowProcess,
		.hInstance = h,
		.lpszClassName = WINDOW_CLASS
	};

	RegisterClassW(&wc);

	// create main window
	return CreateWindowW(
		WINDOW_CLASS,
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
 * all of its children.
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
 * Dispatch messages to the window process. Returns > 0 if a message was received
 * that isn't WM_QUIT, 0 if WM_QUIT was received, and -1 if an error occurred.
 */
static int dispatch(HWND wnd, MSG* msg) {
	// GetMessage doesn't return a bool if it can return -1.
	// According to MS, a boolean has 3 states
	int result = GetMessageW(msg, wnd, 0, 0);

	// check if the message is a dialog message in order
	// to provide keyboard TAB functionality
	if (IsDialogMessageW(wnd, msg)) {
		return 1;
	}

	if (result >= 0) {
		if (result > 0) {
			// not WM_QUIT
			// message received; dispatch and process it
			TranslateMessage(msg);
			DispatchMessage(msg);
		}

		return result;
	}

	// an error occurred
	msgBoxErr(wnd, (int) GetLastError(), L"GetMessageFailed");

	return -1;
}

/**
 * Show an error message box if the thread exited early.
 */
static int threadError(HWND wnd, InstanceData* data) {
	DWORD code = 0;

	data->running = false;

	if (!GetExitCodeThread(data->thread, &code)) {
		msgBoxErr(wnd, ARE_THREAD, L"GetExitCodeThread failed");

		return -1;
	}

	msgBoxErr(wnd, (int) code, L"Thread exited early");

	if (code == ARE_CURL) {
		// if there was a problem with curl, then most likely the user
		// entered something invalid so don't close
		return 1;
	}

	// some other more serious error
	return -1;
}

/**
 * Message and thread handling loop.
 */
static void messageLoop(HWND wnd, InstanceData* data) {
	// result is used to determine the current state
	// < 0: an error occurred
	// = 0: clean exit (user requested termination)
	// > 0: all good, continue processing messages
	int result = 0;
	MSG msg;

	do {
		DWORD waitResult = MsgWaitForMultipleObjectsEx(
			// number of handles in the array below
			// hack: only wait for thread when it's running
			data->running,
			// the handles array
			&data->thread,
			// how long to wait on the thread before returning
			INFINITE,
			// message event type
			QS_ALLEVENTS | QS_ALLPOSTMESSAGE,
			// return as soon as the thread is signalled
			0
		);

		switch (waitResult) {
		case WAIT_OBJECT_0:
			if (!data->running) {
				// when data->running is false (i.e. 0)
				// WAIT_OBJECT_0 is returned when a message
				// should be processed
				result = dispatch(wnd, &msg);
			} else {
				// when data->running is true (i.e. 1)
				// WAIT_OBJECT_0 is returned when the thread
				// has been signalled
				// thread exited early due to an error
				result = threadError(wnd, data);
			}

			break;
		case WAIT_OBJECT_0 + 1:
			// message received when the thread is running
			result = dispatch(wnd, &msg);
			break;
		default:
			// WAIT_ABANDONED_0 to WAIT_ABANDONED_0 + handle count is only
			// ever returned with mutex objects so does not need to be
			// handled here.
			// Only WAIT_FAILED should trigger this code path
			result = -1;
			msgBoxErr(wnd, (int) GetLastError(), L"MsgWaitForMultipleObjects failed");
		}
	} while (result > 0);

	if (result < 0) {
		// error occurred; destroy the window
		DestroyWindow(wnd);
	}
}

/**
 * Initialise, create, and dispatch messages to the main window.
 * @param  h
 * @param  cmdShow
 */
void gui(HINSTANCE h, int cmdShow, InstanceData* data) {
	HWND wnd = init(h, data);

	if (!wnd) {
		msgBoxErr(NULL, ARE_GUI, L"Failed to create window");

		return;
	}

	// create and add the form controls to the main window
	if (!createControls(wnd, &data->handlers)) {
		msgBoxErr(NULL, ARE_GUI, L"Failed to create window controls");

		return;
	}

	// show the window
	ShowWindow(wnd, cmdShow);

	// set the font of the main window and its children to the default system font
	setFont(wnd);

	// message handling
	messageLoop(wnd, data);
}

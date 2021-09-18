#include "gui.h"

/**
 * Copies the bytes from the password input into a useable buffer.
 * @param  data
 * @return      True if all buffers contain at least one byte and false otherwise.
 */
static bool getHandlerText(InstanceData* data) {
	// copy the input from the password handle into a useable buffer
	// GetWindowTextW will truncate and append the the wide null terminator
	// if the input byte count is > FORM_CTRL_BUF_SIZE - 2
	int p = GetWindowTextW(
		data->handlers.ctrlPassword,
		data->password,
		FORM_CTRL_BUF_SIZE
	);

	// GetWindowTextW returns the number of characters copied
	return (p > 0);
}

/**
 * Create the static and edit controls and bind them to the parent window.
 * @param  parent   Used as hWndParent argument to CreateWindow.
 * @param  handlers Members are set to their respective window handlers.
 * @return          True if all windows were created and false otherwise.
 */
static bool createControls(HWND parent, struct formHandlers* handlers) {
	// channel label
	handlers->lblChannel = CreateWindowW(
		L"Static",
		L"Channel",
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

	// channel input
	handlers->ctrlChannel = CreateWindowW(
		L"ComboBox",
		NULL,
		CBS_DROPDOWNLIST | CBS_HASSTRINGS | FORM_CTRL_STYLE,
		MARGIN_X,
		MARGIN_Y + FORM_LBL_H + FORM_MARGIN_H,
		FORM_CTRL_W,
		FORM_CTRL_H,
		parent,
		NULL,
		NULL,
		NULL
	);

	// "refresh" button
	handlers->btnRefresh = CreateWindowW(
		L"Button",
		L"Refresh",
		FORM_BTN_STYLE,
		FORM_BTN_I_MARGIN_X,
		MARGIN_Y + FORM_LBL_H + FORM_MARGIN_H - 1,
		FORM_BTN_I_W,
		FORM_BTN_H,
		parent,
		(HMENU) BTN_REFRESH,
		NULL,
		NULL
	);

	// channel password label
	handlers->lblPassword = CreateWindowW(
		L"Static",
		L"Channel Password",
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

	// channel password input
	handlers->ctrlPassword = CreateWindowW(
		L"Edit",
		NULL,
		FORM_CTRL_STYLE | ES_PASSWORD,
		MARGIN_X,
		MARGIN_Y + FORM_GROUP_H + FORM_LBL_H + FORM_MARGIN_H,
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
		MARGIN_Y + FORM_GROUP_H * 2,
		FORM_BTN_I_W,
		FORM_BTN_H,
		parent,
		(HMENU) BTN_TOGGLE,
		NULL,
		NULL
	);

	// return true if all of the windows were created successfully
	// and false otherwise
	return (
		handlers->lblChannel &&
		handlers->lblPassword &&
		handlers->ctrlChannel &&
		handlers->ctrlPassword &&
		handlers->btnToggle &&
		handlers->btnRefresh
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
 * Creates the thread and waits for it to initialise.
 */
static void startProcedure(HWND wnd, InstanceData* data) {
	// reset the event state
	if (!ResetEvent(data->threadEvent)) {
		msgBoxErr(wnd, ARE_EVENT, L"Could not reset event object");

		return;
	}

	if (!getHandlerText(data)) {
		msgBoxErr(wnd, ARE_USER_INPUT, L"All fields are required");

		return;
	}

	// find channel name in the channel list
	ChannelNode* node = data->chanList->head;
	int idx = (int) SendMessageW(data->handlers.ctrlChannel, CB_GETCURSEL, 0, 0);

	if (idx == CB_ERR) {
		msgBoxErr(wnd, ARE_GUI, L"No item selected in combo box");

		return;
	}

	// advance to the selected channel
	for (int i = 0; i < idx; i++) {
		node = node->next;
	}

	// set the channel ID
	data->channel = node->chan->id;

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
		msgBoxErr(wnd, ARE_THREAD, L"Failed to create thread");

		return;
	}

	// wait for thread initilisation to complete or an initilisation
	// error to be thrown
	HANDLE handles[2] = {data->thread, data->threadEvent};
	DWORD waitResult = WaitForMultipleObjects(
		// how many handles in the array
		(DWORD) 2,
		// the handles to wait on
		handles,
		// wait on all before returning
		false,
		// how long to wait
		INFINITE
	);

	if (waitResult == WAIT_FAILED) {
		msgBoxErr(wnd, ARE_EVENT, L"Waiting for thread initilisation failed");

		return;
	}

	if (waitResult - WAIT_OBJECT_0 == 0) {
		// initialisation error
		 msgBoxErr(wnd, ARE_THREAD, L"Failed to initialise thread");

		 return;
	}

	SetWindowTextW(data->handlers.btnToggle, BTN_STOP);
	data->running = true;
}

/**
 * Stops the running thread.
 */
static void stopProcedure(InstanceData* data) {
	terminateThread(data->thread, data->threadId);
	SetWindowTextW(data->handlers.btnToggle, BTN_START);
	data->running = false;
}

/**
 * Handles the start/stop button clicks.
 */
static LRESULT toggleHandler(HWND wnd, UINT msg, WPARAM w, LPARAM l) {
	(void) msg;
	(void) w;
	(void) l;

	// extract the struct from the window user data
	InstanceData* data = (InstanceData*) GetWindowLongPtr(wnd, GWLP_USERDATA);
	HWND btn = data->handlers.btnToggle;

	// disable the button until the toggle operation has completed
	EnableWindow(btn, false);

	// toggle the state of the program
	if (!data->running) {
		startProcedure(wnd, data);
	} else {
		stopProcedure(data);
	}

	EnableWindow(btn, true);

	return 0;
}

/**
 * Update the combo box with new channels from the API.
 */
static int refreshChannels(InstanceData* data) {
	// get all channels from the API
	cJSON* array;
	int result = getChannelsJSON(&array);
	HWND cb = data->handlers.ctrlChannel;

	if (result != 0) {
		return result;
	}

	// clear any items currently in the combo box
	SendMessageW(cb, CB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);

	// create a channel list from the array
	ChannelList* list = channelListFromJSON(array);

	// no longer need the JSON array
	cJSON_Delete(array);

	if (!list) {
		return ARE_OUT_OF_MEM;
	}

	// free the current channel list and replace it with the new one
	freeChannelList(data->chanList);
	data->chanList = list;

	// send all the channel names to the combobox
	for (ChannelNode* node = list->head; node; node = node->next) {
		// channel names are already sorted, so just append
		LRESULT smr = SendMessageW(
			cb,
			CB_INSERTSTRING,
			(WPARAM) -1,
			(LPARAM) node->chan->name);

		if (smr == CB_ERR || smr == CB_ERRSPACE) {
			return ARE_GUI;
		}
	}

	// pre-select the first channel in the list
	SendMessageW(cb, CB_SETCURSEL, (WPARAM) 0, (LPARAM) 0);

	return 0;
}

/**
 * Handle refresh button clicks.
 */
static LRESULT refreshHandler(HWND wnd, UINT msg, WPARAM w, LPARAM l) {
	(void) msg;
	(void) w;
	(void) l;

	InstanceData* data = (InstanceData*) GetWindowLongPtr(wnd, GWLP_USERDATA);

	// disable the button while the update occurs
	EnableWindow(data->handlers.btnRefresh, false);

	int result = refreshChannels(data);

	if (result != 0) {
		msgBoxErr(wnd, result, L"Failed to update channels");
	}

	// re-enable the button
	EnableWindow(data->handlers.btnRefresh, true);

	return 0;
}

struct buttonHandler {
	WPARAM id;
	LRESULT (*handler)(HWND, UINT, WPARAM, LPARAM);
};

static const struct buttonHandler btnHandlers[] = {
	{BTN_TOGGLE, toggleHandler},
	{BTN_REFRESH, refreshHandler}
};

static const size_t btnHandlersLen = sizeof(btnHandlers) / sizeof(btnHandlers[0]);

/**
 * Handles WM_COMMAND.
 */
static LRESULT wmCommand(HWND wnd, UINT msg, WPARAM w, LPARAM l) {
	for (int i = 0; i < btnHandlersLen; i++) {
		if (w == btnHandlers[i].id) {
			return btnHandlers[i].handler(wnd, msg, w, l);
		}
	}

	return DefWindowProcW(wnd, msg, w, l);
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
	int result = GetMessageW(msg, NULL, 0, 0);

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
	wprintf(L"%d: GetMessageFailed()\n", (int) GetLastError());

	return -1;
}

/**
 * Show an error message box if the thread exited early.
 */
static int threadError(InstanceData* data) {
	DWORD code = 0;

	data->running = false;

	if (!GetExitCodeThread(data->thread, &code)) {
		wprintf(L"%d: GetExitCodeThread failed\n", ARE_THREAD);

		return -1;
	}

	wprintf(L"%d: Thread exited early\n", (int) code);

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
				result = threadError(data);
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
			wprintf(L"%d: MsgWaitForMultipleObjectsEx failed", (int) GetLastError());
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

	// get the channels from the API and add them to the combobox
	int result = refreshChannels(data);

	if (result != 0) {
		msgBoxErr(NULL, result, L"Failed to get channels from API");

		return;
	}

	// show the window
	ShowWindow(wnd, cmdShow);

	// set the font of the main window and its children to the default system font
	setFont(wnd);

	// message handling
	messageLoop(wnd, data);
}

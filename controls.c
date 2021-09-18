#include "controls.h"

/**
 * Copies the bytes from the password input into a useable buffer.
 * @param  data
 * @return      True if all buffers contain at least one byte and false otherwise.
 */
bool getHandlerText(InstanceData* data) {
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
bool createControls(HWND parent, struct formHandlers* handlers) {
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

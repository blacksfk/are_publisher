#ifndef INSTANCE_DATA_H
#define INSTANCE_DATA_H

#include "shared_mem.h"

// how much text could you really enter?? 4096 wchar_t characters should be plenty...
#define FORM_CTRL_BUF_SIZE 8192

// struct that groups the window handlers of the form controls
struct formHandlers {
	HWND lblAddress;
	HWND lblChannel;
	HWND lblPassword;

	HWND ctrlAddress;
	HWND ctrlChannel;
	HWND ctrlPassword;

	HWND btnSet;
	HWND btnRefresh;
	HWND btnToggle;
};

typedef struct instanceData {
	// text input buffers
	wchar_t* address;
	wchar_t* channel;
	wchar_t* password;

	// whether or not the end user has clicked the start button
	bool running;

	// shared memory object
	SharedMem* sm;

	// processing thread handle
	HANDLE thread;

	// processing thread identifier
	DWORD threadId;

	// as described above
	struct formHandlers handlers;

	// processing thread init signal
	HANDLE threadEvent;
} InstanceData;

InstanceData* createInstanceData(SharedMem* sm);
void freeInstanceData(InstanceData* data);

#endif

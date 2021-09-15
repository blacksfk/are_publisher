#ifndef INSTANCE_DATA_H
#define INSTANCE_DATA_H

#include "channel.h"
#include "shared_mem.h"

// how much text could you really enter?? 4096 wchar_t characters should be plenty...
#define FORM_CTRL_BUF_SIZE 8192

// struct that groups the window handlers of the form controls
struct formHandlers {
	HWND lblChannel;
	HWND lblPassword;

	HWND ctrlChannel;
	HWND ctrlPassword;

	HWND btnRefresh;
	HWND btnToggle;
};

typedef struct instanceData {
	// pointer to the selected channel's ID
	// niether allocated nor de-allocated by instance data functions
	wchar_t* channel;

	// text input copied from the password window handler
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

	// channel list
	ChannelList* chanList;
} InstanceData;

InstanceData* createInstanceData(SharedMem* sm, ChannelList* list);
void freeInstanceData(InstanceData* data);

#endif

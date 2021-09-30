#ifndef INSTANCE_DATA_H
#define INSTANCE_DATA_H

#include "channel.h"
#include "shared_mem.h"

// struct that groups the window handlers of the form controls
struct formHandlers {
	HWND lblChannel;
	HWND lblPassword;

	HWND ctrlChannel;
	HWND ctrlPassword;

	HWND btnRefresh;
	HWND btnToggle;
	HWND btnInfo;
};

typedef struct instanceData {
	// selected channel's ID copied and converted from the channel list
	char* channel;

	// text input copied (and converted) from the password window handler
	char* password;

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

InstanceData* createInstanceData(SharedMem* sm);
void freeInstanceData(InstanceData* data);

#endif

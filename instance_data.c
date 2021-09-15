#include "instance_data.h"

/**
 * Allocate memory for an instance data struct and initialise various members.
 * @param  sm
 * @return    Returns NULL if memory could not be allocated.
 */
InstanceData* createInstanceData(SharedMem* sm) {
	InstanceData* data = malloc(sizeof(*data));

	if (!data) {
		// out of memory
		return NULL;
	}

	// create event to be signalled by the thread when it is ready
	data->threadEvent = CreateEventW(NULL, true, false, L"ThreadInitEvent");

	if (!data->threadEvent) {
		// could not create event object
		freeInstanceData(data);

		return NULL;
	}

	// allocate memory for text input controls
	data->password = malloc(FORM_CTRL_BUF_SIZE);

	if (!data->password) {
		// out of memory
		freeInstanceData(data);

		return NULL;
	}

	// calculate the number of characters (should be FORM_CTRL_BUF_SIZE / 2)
	size_t count = FORM_CTRL_BUF_SIZE / sizeof(wchar_t);

	// initialise all text input buffers with wchar_t null bytes
	wmemset(data->password, L'\0', count);

	data->sm = sm;
	data->chanList = NULL;
	data->running = false;
	data->thread = NULL;
	data->threadId = 0;
	data->channel = NULL;

	return data;
}

/**
 * Free instance data along with all heap allocated members. Does nothing if data is NULL.
 */
void freeInstanceData(InstanceData* data) {
	if (!data) {
		return;
	}

	freeChannelList(data->chanList);
	free(data->password);
	free(data);
}

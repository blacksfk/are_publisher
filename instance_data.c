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

	data->sm = sm;
	data->chanList = NULL;
	data->running = false;
	data->thread = NULL;
	data->threadId = 0;
	data->channel = NULL;
	data->password = NULL;

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
	free(data->channel);
	free(data->password);
	free(data);
}
